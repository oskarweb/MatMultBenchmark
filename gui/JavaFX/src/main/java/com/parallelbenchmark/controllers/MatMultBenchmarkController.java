package com.parallelbenchmark.controllers;

import com.parallelbenchmark.MatMultBenchmarkResult;
import com.parallelbenchmark.utils.IOUtils;

import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ListChangeListener;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.concurrent.Task;
import javafx.scene.chart.BarChart;
import javafx.scene.chart.CategoryAxis;
import javafx.scene.chart.XYChart;

import java.io.*;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MatMultBenchmarkController {
    @FXML
    private Button runBenchmarkButton;

    @FXML 
    private TableView<MatMultBenchmarkResult> resultTable;

    @FXML
    private BarChart<String, Number> barChart;

    @FXML
    private CategoryAxis xAxis;

    private final ObservableList<MatMultBenchmarkResult> benchmarkResults = FXCollections.observableArrayList();
    private final Map<String, XYChart.Series<String, Number>> barChartSeriesMap = new HashMap<>();

    @FXML
    public void initialize() 
    {
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
        resultTable.setItems(benchmarkResults);
        
        benchmarkResults.addListener((ListChangeListener<MatMultBenchmarkResult>) change -> {
            while (change.next()) {
                if (change.wasAdded()) {
                    for (MatMultBenchmarkResult result : change.getAddedSubList()) {
                        String category = result.getMatrixDims() + " (" + result.getDataType() + ")";
                        String seriesKey = result.getName();

                        if (!xAxis.getCategories().contains(category)) {
                            xAxis.getCategories().add(category);
                        }
                    
                        XYChart.Series<String, Number> series = barChartSeriesMap.computeIfAbsent(seriesKey, key -> {
                            XYChart.Series<String, Number> s = new XYChart.Series<>();
                            s.setName(key);
                            barChart.getData().add(s);
                            return s;
                        });
                    
                        XYChart.Data<String, Number> data = new XYChart.Data<>(category, result.getAvgExecutionTimeSeconds());
                        series.getData().add(data);
                    }
                }
            }
        });

        barChart.setAnimated(false);

        runBenchmarkButton.setOnAction(event -> runBenchmark());
    }

    public void runBenchmark() 
    {
        Platform.runLater(() -> {
            clearResults();
            runBenchmarkButton.setText("Running...");
            runBenchmarkButton.setDisable(true);
        });

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

                    IOUtils.readJsonObjects(
                        process.getInputStream(),
                        MatMultBenchmarkResult.class,
                        result -> Platform.runLater(() -> benchmarkResults.add(result))
                    );
         
                    int exitCode = process.waitFor();
                    if (exitCode != 0) {
                        throw new RuntimeException("Benchmark process exited with status " + exitCode);
                    }
                } catch (IOException | InterruptedException e) {
                    e.printStackTrace();
                } catch (RuntimeException e) {
                    System.err.println("Error: " + e.getMessage());
                } finally {
                    Platform.runLater(() -> {
                        runBenchmarkButton.setText("Run Benchmark");
                        runBenchmarkButton.setDisable(false);
                    });
                }
                return null;
            }
        };
        new Thread(task).start();
    }

    private void clearResults()
    {
        benchmarkResults.clear();
        barChart.getData().clear();
        xAxis.getCategories().clear();
        barChartSeriesMap.clear();
    }
}