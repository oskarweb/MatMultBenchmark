module matmultbench {
    requires javafx.controls;
    requires javafx.fxml;
    requires com.google.gson;

    opens com.matmultbench to javafx.fxml;

    exports com.matmultbench;
}