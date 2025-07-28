package com.parallelbenchmark;

public class BenchmarkResult  
{
    public String name;
    public int times_executed;
    public double avg_execution_time_seconds;

    public BenchmarkResult() {}

    public String getName() { return name; }
    public int getTimesExecuted() { return times_executed; }
    public double getAvgExecutionTimeSeconds() { return avg_execution_time_seconds; }
}