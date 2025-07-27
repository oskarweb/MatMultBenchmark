package com.matmultbench;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import java.io.*;
import java.util.Arrays;
import java.util.List;
import com.google.gson.Gson;

public class Controller {

    @FXML
    private Button runBenchmarkButton;

    @FXML
    public void initialize() {
        runBenchmarkButton.setOnAction(event -> runBenchmark());
    }

    private void runBenchmark() {
        try {
            File exe = new File("build/benchmarks/bin/OclDemo.exe");
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
            List<MatMultBenchmarkResult> results = Arrays.asList(resultArray);
            results.forEach(System.out::println);
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        } catch (RuntimeException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}