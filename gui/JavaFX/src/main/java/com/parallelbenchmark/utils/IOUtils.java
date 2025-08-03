package com.parallelbenchmark.utils;

import com.google.gson.Gson;
import com.parallelbenchmark.MatMultBenchmarkResult;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;
import java.util.function.Consumer;

public final class IOUtils {
    private IOUtils() {
        throw new UnsupportedOperationException("Utility class");
    }

    public static <T> void readJsonObjects(InputStream inputStream, Class<T> clazz, Consumer<T> callback) throws IOException {
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
                        callback.accept(result);
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