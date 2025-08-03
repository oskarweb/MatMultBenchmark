package com.parallelbenchmark.controllers;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.layout.StackPane;
import javafx.scene.control.ListView;

import java.io.IOException;

public class MainController {
    @FXML private ListView<String> menuList;
    @FXML private StackPane contentPane;

    @FXML
    public void initialize() {
        menuList.getItems().addAll("Matrix Multiplication", "Test");
        menuList.getSelectionModel().selectedItemProperty().addListener((obs, oldVal, newVal) -> {
            switchView(newVal);
        });

        switchView("Matrix Multiplication");
    }

    private void switchView(String viewName) {
        String fxmlFile = switch (viewName) {
            case "Test" -> "/com/parallelbenchmark/TestView.fxml";
            default -> "/com/parallelbenchmark/MatrixMultView.fxml";
        };

        try {
            Parent view = FXMLLoader.load(getClass().getResource(fxmlFile));
            contentPane.getChildren().setAll(view);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }    
}
