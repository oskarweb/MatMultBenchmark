package com.matmultbench;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import java.io.*;

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

            BufferedReader reader = new BufferedReader(
                    new InputStreamReader(process.getInputStream())
            );

            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println("Process Output: " + line);
            }

            int exitCode = process.waitFor();
            System.out.println("Process exited with code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }
}