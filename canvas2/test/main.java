// package src;

import java.io.*;
import src.canvas;
// import java.net.URL;
// import java.net.HttpURLConnection;


public class main {
	public static void main(String[] args) {
		canvas c = new canvas();
		// String courses = c.getCourse();
		canvas.readToken();
		canvas.getID();
		canvas.fetchCourseList();
		canvas.fetchCourses();

		// canvas.createInfoDatabase();

		// System.out.println(System.getProperty("user.dir"));
	}
}