package com.matmultbench;

public class MatMultBenchmarkResult extends BenchmarkResult 
{
    public String data_type;
    public String matrix_dims;

    @Override
    public String toString()
    {
        return name + " (type: " + data_type + ", matrix_dims: " + matrix_dims + ") - " + avg_execution_time_seconds + "s";
    }
}