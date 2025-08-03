package com.parallelbenchmark.controllers;

import com.parallelbenchmark.MatMultBenchmarkResult;
import com.parallelbenchmark.utils.IOUtils;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.concurrent.Task;

import java.io.*;
import java.util.Arrays;
import java.util.List;

public class MatMultBenchmarkController {
    @FXML
    private Button runBenchmarkButton;

    @FXML 
    private TableView<MatMultBenchmarkResult> resultTable;

    @FXML
    public void initialize() {
        resultTable.setColumnResizePolicy(TableView.CONSTRAINED_RESIZE_POLICY_FLEX_LAST_COLUMN);
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
        resultTable.getColumns().setAll(columns);

        runBenchmarkButton.setOnAction(event -> runBenchmark());
    }

    public void runBenchmark() {
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

                    ObservableList<MatMultBenchmarkResult> benchmarkResults = FXCollections.observableArrayList();
                    resultTable.setItems(benchmarkResults);

                    IOUtils.writeJsonObjectsToList(process.getInputStream(), benchmarkResults, MatMultBenchmarkResult.class);
                    
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