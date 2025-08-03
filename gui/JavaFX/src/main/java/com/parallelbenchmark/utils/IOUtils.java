package com.parallelbenchmark.utils;

import javafx.collections.ObservableList;
import com.google.gson.Gson;
import com.parallelbenchmark.MatMultBenchmarkResult;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public final class IOUtils {
    private IOUtils() {
        throw new UnsupportedOperationException("Utility class");
    }

    public static <T> void writeJsonObjectsToList(InputStream inputStream, ObservableList<T> dstList, Class<T> clazz) throws IOException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
        Gson gson = new Gson();
        String line;
        StringBuilder objectBuilder = new StringBuilder();
        int braceDepth = 0;
        boolean insideObject = false;
        while ((line = reader.readLine()) != null) {
            line = line.trim();
            if (line.isEmpty()) continue;
        
            for (char c : line.toCharArray()) {
                if (c == '{') {
                    if (!insideObject) {
                        insideObject = true;
                    }
                    braceDepth++;
                } else if (c == '}') {
                    braceDepth--;
                }
            }
        
            if (insideObject) {
                objectBuilder.append(line).append("\n");
            
                if (braceDepth == 0) {
                    String jsonObject = objectBuilder.toString();
                    objectBuilder.setLength(0);
                    insideObject = false;
                
                    try {
                        T result = gson.fromJson(jsonObject, clazz);
                        dstList.add(result);
                    } catch (Exception e) {
                        System.err.println("Failed to parse JSON object:");
                        System.err.println(jsonObject);
                        e.printStackTrace();
                    }
                }
            }
        }
    }
}