package src;

import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.util.*;
import java.util.regex.*;
import java.sql.*;
// import com.google.common.base.*;


public class canvas {
	private static ArrayList<String> courses;
	private static ArrayList<String> courses_id;
	private static String token;
	private static String student_id;
	private static String getURL(String url_string) {
		String content = "", line = "";
		try {
			URL url = new URL(url_string);
			HttpURLConnection http = (HttpURLConnection)url.openConnection();
			http.setRequestMethod("GET");
			http.connect();
			BufferedReader reader = new BufferedReader(new InputStreamReader(http.getInputStream()));
			while((line = reader.readLine()) != null)
				content += line;
			reader.close();
			return content;
		}
		catch(Exception e) {
			System.out.println(e.getMessage());
			return "error";
		}

	}

	public static void readToken() {
		File file = new File("token.txt");
		BufferedReader br = null;
		try {
			try {
				br = new BufferedReader(new FileReader(file));
			}
			catch (Exception e) {
				System.out.println(e.getMessage());
			}
			String text;
			try {
				token = br.readLine();
			}
			catch (Exception e) {
				System.out.println(e.getMessage());
			}
		} finally {
			if (br != null) {
				try {
					br.close();
				}
				catch (Exception e) {
					System.out.println(e.getMessage());
				}
			}
		}
	}

	public static void getID() {
		String content = getURL("https://umjicanvas.com/api/v1/users/self?access_token=" + token);
		// System.out.println(content);
		Matcher m = Pattern.compile("\"id\":(\\d+)").matcher(content);
		if (m.find()) {
			student_id = m.group(1);
			// System.out.println(student_id);
		}
	}

	public static void fetchCourseList() {
		String content = getURL("https://umjicanvas.com/api/v1/users/self/favorites/courses?access_token=" + token);
		Matcher m = Pattern.compile("\"course_code\":\"(.*?)\"").matcher(content);
		Matcher m_id = Pattern.compile("\"id\":(\\d+)").matcher(content);
		while (m.find() && m_id.find()) {
			Matcher m1 = Pattern.compile("Entry|FOCS|Undergraduate").matcher(m.group());
			if (!m1.find()) {
				// System.out.println(m.group(1));
				courses_id.add(m_id.group(1));
				courses.add(m.group(1));
			}

		}
		// System.out.println(courses_id.toString());

	}

	public static void fetchCourses() {
		for (int i = 0; i < courses.size(); i ++) {
			String course = courses.get(i);
			String course_id = courses_id.get(i);
			String content = null;
			System.out.println("Fetching data from " + course);
			for (int j = 1; j <= 5; j ++) {
				String web_content = getURL("https://umjicanvas.com/api/v1/courses/"
					+ course_id +"/users?access_token="
					+ token +"&include[]=email&include[]=enrollments&per_page=50&page="
					+ j);
				if (web_content.length() < 10) {
					// System.out.println("Break at " + j);
					break;
				}
				content = content + web_content;
			}
			// Matcher m;
			// m = Pattern.compile("\"user_id\":(\\d+)").matcher(content);
			// while (m.find()) {
			// 	System.out.println(m.group(1));
			// }
			// Map<String, String> map = Splitter.on(",").withKeyValueSeparator(":").split(content);
			// for (String key: map.keySet()){  
			// 	System.out.println(key+ " = " + map.get(key));
			// }
		}
	}

	public static void createInfoDatabase() {
        Connection connection = null;
        Statement statement = null;
        ArrayList<String> requests = new ArrayList<String> ();
        try {
            Class.forName("org.sqlite.JDBC");

            connection = DriverManager.getConnection("jdbc:sqlite:data/data.db");
            statement = connection.createStatement();
            requests.add("create table ECE4710J_info("
            	+ "id integer, "
            	+ "name text, "
            	+ "sortable_name text, "
            	+ "short_name text, "
            	+ "enrollment_id integer, "
            	+ "type text, "
            	+ "created_at text, "
            	+ "updated_at text, "
            	// + "associated_user_id integer, "
            	+ "start_at text, "
            	+ "end_at text, "
            	+ "course_section_id integer, "
            	+ "root_account_id integer, "
            	+ "limit_privileges_to_course_section text, "
            	+ "enrollment_state text, "
            	+ "role text, "
            	+ "role_id integer, "
            	+ "last_activity_at text, "
            	+ "total_activity_time integer, "
            	+ "grades text, "
            	+ "html_url text, "
            	+ "email text"
            	+ ")");
            connection.setAutoCommit(false);
            for (int i = 0; i < requests.size(); i ++) {
            	statement.addBatch(requests.get(i));
            }
            statement.executeBatch();
            connection.commit();
            System.out.println("Batch executed");
            // String sql = "create table test(id integer, name text)";
            // String sql = "insert into test values(4416, \"Yang Yiwen\")";
			// statement.executeQuery(sql);
            // String sql = "select * from test";
            // String sql = "drop table test";
            // ResultSet rs = statement.executeQuery(sql);
            // statement.executeQuery(sql);
            // while (rs.next()) {
            //     System.out.println(rs.getString(2));
            // }
        } catch (ClassNotFoundException e) {
          e.printStackTrace();
        } catch (SQLException e) {
          e.printStackTrace();
        } finally {
            try {
                if (statement != null) {
                    statement.close();
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
            try {
                if (connection != null) {
                    connection.close();
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    
    }

	public canvas() {
		courses = new ArrayList<String>();
		courses_id = new ArrayList<String>();
	}
}