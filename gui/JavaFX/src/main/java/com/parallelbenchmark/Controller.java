package com.parallelbenchmark;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.concurrent.Task;

import java.io.*;
import java.util.ArrayList;
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
        Task<Void> task = new Task<>() {
            @Override
            protected Void call() {
                try {
                    File exe = new File("build/benchmarks/bin/parallel_benchmark.exe");
                    File workingDir = new File("build/benchmarks/bin");

                    ProcessBuilder builder = new ProcessBuilder(exe.getAbsolutePath());
                    builder.directory(workingDir);
                    builder.redirectErrorStream(true);

                    Process process = builder.start();

                    BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));

                    ObservableList<MatMultBenchmarkResult> benchmarkResults = FXCollections.observableArrayList();
                    table.setItems(benchmarkResults);

                    Gson gson = new Gson();
                    String line;
                    StringBuilder objectBuilder = new StringBuilder();
                    int braceDepth = 0;
                    boolean insideObject = false;

                    while ((line = reader.readLine()) != null) {
                        line = line.trim();
                        if (line.isEmpty()) continue;
                    
                        for (char c : line.toCharArray()) {
                            if (c == '{') {
                                if (!insideObject) {
                                    insideObject = true;
                                }
                                braceDepth++;
                            } else if (c == '}') {
                                braceDepth--;
                            }
                        }
                    
                        if (insideObject) {
                            objectBuilder.append(line).append("\n");
                        
                            if (braceDepth == 0) {
                                String jsonObject = objectBuilder.toString();
                                objectBuilder.setLength(0);
                                insideObject = false;
                            
                                try {
                                    MatMultBenchmarkResult result = gson.fromJson(jsonObject, MatMultBenchmarkResult.class);
                                    table.getItems().add(result);
                                } catch (Exception e) {
                                    System.err.println("Failed to parse JSON object:");
                                    System.err.println(jsonObject);
                                    e.printStackTrace();
                                }
                            }
                        }
                    }
                
                    int exitCode = process.waitFor();
                    if (exitCode != 0) {
                        throw new RuntimeException("Benchmark process exited with status " + exitCode);
                    }
                
                } catch (IOException | InterruptedException e) {
                    e.printStackTrace();
                } catch (RuntimeException e) {
                    System.err.println("Error: " + e.getMessage());
                }
                return null;
            }
        };
        new Thread(task).start();
    }
}