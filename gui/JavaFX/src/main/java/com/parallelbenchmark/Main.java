package com.parallelbenchmark;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class Main extends Application {
    @Override
    public void start(Stage primaryStage) throws Exception {
        Parent root = FXMLLoader.load(getClass().getResource("/com/parallelbenchmark/main.fxml"));
        primaryStage.setTitle("JavaFX Process Launcher");
        primaryStage.setScene(new Scene(root, 1000, 800));
        primaryStage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}