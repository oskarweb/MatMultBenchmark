package com.parallelbenchmark;

public class MatMultBenchmarkResult extends BenchmarkResult 
{
    public String data_type;
    public String matrix_dims;

    public MatMultBenchmarkResult() {}

    @Override
    public String toString()
    {
        return name + " (type: " + data_type + ", matrix_dims: " + matrix_dims + ") - " + avg_execution_time_seconds + "s";
    }

    public String getDataType() { return data_type; }
    public String getMatrixDims() { return matrix_dims; }
}