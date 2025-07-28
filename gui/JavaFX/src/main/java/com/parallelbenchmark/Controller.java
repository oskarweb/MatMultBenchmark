package com.parallelbenchmark;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;

import java.io.*;
import java.util.Arrays;
import java.util.List;
import com.google.gson.Gson;

public class Controller {

    @FXML
    private Button runBenchmarkButton;

    @FXML 
    private TableView<MatMultBenchmarkResult> table;

    @FXML
    public void initialize() {
        table.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY_FLEX_LAST_COLUMN);
        TableColumn<MatMultBenchmarkResult, String> nameCol = new TableColumn<>("Name");
        nameCol.setCellValueFactory(new PropertyValueFactory<>("name"));
        TableColumn<MatMultBenchmarkResult, Integer> timesCol = new TableColumn<>("Times Executed");
        timesCol.setCellValueFactory(new PropertyValueFactory<>("timesExecuted"));
        TableColumn<MatMultBenchmarkResult, Double> avgTimeCol = new TableColumn<>("Avg Time (s)");
        avgTimeCol.setCellValueFactory(new PropertyValueFactory<>("avgExecutionTimeSeconds"));
        TableColumn<MatMultBenchmarkResult, String> dataTypesCol = new TableColumn<>("Data Type");
        dataTypesCol.setCellValueFactory(new PropertyValueFactory<>("dataType"));
        TableColumn<MatMultBenchmarkResult, String> matrixDimsCol = new TableColumn<>("Matrix Dims");
        matrixDimsCol.setCellValueFactory(new PropertyValueFactory<>("matrixDims"));
        
        List<TableColumn<MatMultBenchmarkResult, ?>> columns = Arrays.asList(nameCol, timesCol, avgTimeCol, dataTypesCol, matrixDimsCol);
        table.getColumns().setAll(columns);

        runBenchmarkButton.setOnAction(event -> runBenchmark());
    }

    private void runBenchmark() {
        try {
            File exe = new File("build/benchmarks/bin/parallel_benchmark.exe");
            File workingDir = new File("build/benchmarks/bin");

            ProcessBuilder builder = new ProcessBuilder(exe.getAbsolutePath());
            builder.directory(workingDir);
            builder.redirectErrorStream(true);

            Process process = builder.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder jsonBlock = new StringBuilder();

            boolean inJson = false;
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();

                if (line.equals("Running benchmarks")) {
                    inJson = true;
                    continue;
                }

                if (line.startsWith("STATUS:")) {
                    int status = process.waitFor();
                    if (status != 0) {
                        throw new RuntimeException("Benchmark failed with STATUS: " + status);
                    }
                    break;
                }

                if (inJson) {
                    jsonBlock.append(line).append("\n");
                }
            }

            System.out.println("Raw JSON block:");
            System.out.println(jsonBlock);

            Gson gson = new Gson();
            MatMultBenchmarkResult[] resultArray = gson.fromJson(jsonBlock.toString(), MatMultBenchmarkResult[].class);
            ObservableList<MatMultBenchmarkResult> benchmarkResults = FXCollections.observableArrayList(resultArray);
            table.setItems(benchmarkResults);
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        } catch (RuntimeException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}