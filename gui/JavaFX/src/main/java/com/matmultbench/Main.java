package com.matmultbench;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;

public class Main extends Application {
    @Override
    public void start(Stage stage) {
        Label label = new Label("Test");
        StackPane root = new StackPane(label);
        Scene scene = new Scene(root, 400, 300);
        stage.setScene(scene);
        stage.setTitle("Matrix Multiplication Benchmark");
        stage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}