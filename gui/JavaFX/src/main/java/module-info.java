module matmultbench {
    requires javafx.controls;
    requires javafx.fxml;

    opens com.matmultbench to javafx.fxml;

    exports com.matmultbench;
}