module parallelbenchmark {
    requires javafx.controls;
    requires javafx.fxml;
    requires com.google.gson;

    opens com.parallelbenchmark to javafx.fxml;

    exports com.parallelbenchmark;
}