import pandas as pd
import numpy as np
import json
import argparse
import datetime
import os
import sys
import warnings
import contribution
from oauthlib.oauth2.rfc6749 import tokens
from oauthlib.oauth2 import Server

def warn(*args, **kwargs):
	pass
warnings.warn = warn

def readName():
	names = pd.DataFrame()
	with open("./VE370.txt", "r") as f:
		for item in f:
			if ((item == "\n") | (item[0] == "#")):
				continue
			row = json.loads(item)
			df = pd.DataFrame(row, index = [0])
			names = names.append(df, ignore_index = True)
	print(names)

def findMyCourse():
	with open("/tmp/temp.txt", "r") as f:
		courses = f.readline()
	courses = courses.replace("null", "None")
	courses = courses.replace("false", "False")
	courses = courses.replace("true", "True")
	courses = eval(courses)
	li = [courses[0]["enrollments"][0]["user_id"]]
	for item in courses:
		course_id = item["id"]
		course_name = item["course_code"]
		if ((course_id == 7) | (course_id == 1170) | (course_id == 1267)):
			continue
		li.append(course_id)
		li.append(course_name)
	return li

def readCourse():
	courses = pd.DataFrame()
	with open("./courses.txt", "r") as f:
		for item in f:
			if ((item == "\n") | (item[0] == "#")):
				continue
			row = json.loads(item)
			df = pd.DataFrame(row, index = [0])
			courses = pd.concat([courses, df], ignore_index = True)
	print(courses)

def validate_client_id(self, client_id, request):
    request.claims = {
        'aud': self.client_id
    }
    return True

def decode():
	private_pem_key = 12345
	validator = 12345

	server = Server(
	  validator,
	  token_generator = tokens.signed_token_generator(private_pem_key, issuer="foobar")
	)
	print(server)

def readUsers(course_name):
	course_raw_data = "/tmp/'" + course_name + "'.txt"
	with open(course_raw_data, "r") as f:
		li = f.readlines()
	for i in range(len(li)):
		li[i] = li[i].replace("null", "None")
		li[i] = li[i].replace("false", "False")
		li[i] = li[i].replace("true", "True")
		li[i] = eval(li[i])
	df = pd.DataFrame(li[0][0])
	for i in range(len(li)):
		for j in range(len(li[i])):
			if ((i == 0) & (j == 0)):
				continue
			df = df.append(li[i][j], ignore_index = True)
	return df

def extractEnroll(row, string):
	row = row["enrollments"]
	if (isinstance(row, list)):
		row = row[0]
	try:
		row[string]
	except KeyError:
		return None
	return (row[string])

def main():
	# readName()
	# readCourse()
	# decode()

	# Parse arguments
	parser = argparse.ArgumentParser(prog = "canvas")
	parser.add_argument("course", help = "Input course name that you want to operate on.", type = str)
	parser.add_argument("-f", "--favorite", help = "Return a list of current courses.", action = "store_true")
	parser.add_argument("-m", "--mine", help = "Query my activity, requires id.", type = int)
	parser.add_argument("-s", "--summary", help = "Save today's overall activity, requires the amount of activity.", type = int)
	parser.add_argument("-r", "--rank", help = "Rank users daily activity in the course.", action = "store_true")
	parser.add_argument("-p", "--plot", help = "Plot my contribution.", nargs = "?", const = "./")
	parser.add_argument("-q", "--query", help = "Query others' contribution, requires id.", type = int)
	parser.add_argument("-t", "--test", help = "Test.", type = str)
	parser.add_argument("-d", "--directory", help = "Specify a running directory.", nargs = "?", const = "./")
	parser.add_argument("-c", "--check", help = "Check whether to write back the cache.", action = "store_true")
	args = parser.parse_args()
	args.course = args.course.strip("'")

	if (args.test):
		args.test = args.test.strip("'")
		print(args.test)
		sys.exit(0)

	if (args.directory):
		png_save_path = args.directory + "img/"
		data_path = args.directory + "data/"
	else:
		png_save_path = "./img/"
		data_path = "./data/"
	cache_path = os.path.join(data_path, ".cache")
	if not (os.path.exists(data_path)):
		os.makedirs(data_path)
	if not (os.path.exists(cache_path)):
		os.makedirs(cache_path)
	today_cache_path = os.path.join(cache_path, datetime.date.today().isoformat())
	yesterday_cache_path = os.path.join(cache_path, (datetime.date.today() - datetime.timedelta(1)).isoformat())
	ototoi_cache_path = os.path.join(cache_path, (datetime.date.today() - datetime.timedelta(2)).isoformat())

	if (args.check):
		if not (os.path.exists(today_cache_path)):
			os.makedirs(today_cache_path)
			if (os.path.exists(yesterday_cache_path)):
				print("Writing back cache...")
				filenames = next(os.walk(yesterday_cache_path, (None, None, [])))[2]
				for filename in filenames:
					df_cache = pd.read_csv(os.path.join(yesterday_cache_path, filename), compression = "gzip", index_col = 0)
					df = pd.read_csv(os.path.join(data_path, filename), compression = "gzip", index_col = 0)
					# print(df_cache[df_cache["student_id"] == 4518]["total_activity_time"])
					df = pd.merge(df, df_cache[["student_id", "total_activity_time"]], left_on = "id", right_on = "student_id")
					df = df.rename(columns = {"total_activity_time": (datetime.date.today() - datetime.timedelta(1)).isoformat()})
					df = df.drop("student_id", axis = 1)
					df.to_csv(os.path.join(data_path, filename), compression = "gzip")
					# os.remove(os.path.join(yesterday_cache_path, filename))
					# print(df[df["id"] == 4518][(datetime.date.today() - datetime.timedelta(1)).isoformat()])
			if (os.path.exists(ototoi_cache_path)):
				filenames = next(os.walk(ototoi_cache_path, (None, None, [])))[2]
				for filename in filenames:
					os.remove(os.path.join(ototoi_cache_path, filename))
				os.rmdir(ototoi_cache_path)

		print("Cache check Done.")
		sys.exit(0)

	

	course_csv = os.path.join(today_cache_path, args.course + ".csv.gz")
	activity_csv = os.path.join(data_path, "my_activity.csv")
	if not (os.path.exists(activity_csv)):
		# os.system("touch " + activity_csv)
		with open(activity_csv, "w") as f:
			f.write("date,activity\n")

	if (args.summary):
		with open(activity_csv, "r") as f:
			my_activities = f.readlines()

		last_updated = my_activities[-1].split(",")[0]
		if (last_updated == datetime.date.today().isoformat()):
			with open(activity_csv, "w") as f:
				for i in range(len(my_activities) - 1):
					f.write(my_activities[i])
		with open(activity_csv, "a") as f:
			f.write("{},{}\n".format(datetime.date.today().isoformat(), args.summary))
			sys.exit(0)

	if (args.plot):
		df = pd.read_csv(activity_csv)
		activity = df["activity"].values
		delta = [0]
		len_df = len(df)
		if (len_df == 0):
			print("There is no activity data yet. Try run.sh first.")
			sys.exit(0)
		elif (len_df == 1):
			print("Past data not found, please try again tomorrow.")
			sys.exit(0)
		for i in range(len_df - 1):
			delta.append(activity[i + 1] - activity[i])
		contribution.contributionPlot(delta, by = "month", save = png_save_path)
		# print(delta)
		sys.exit(0)

	if (args.query):
		query_data = []
		query_csv = os.path.join(data_path, args.course + ".csv.gz")
		if not (os.path.exists(query_csv)):
			if not (os.path.exists(os.path.join(today_cache_path))):
				print("There is no activity data yet. Try run.sh first.")
				sys.exit(0)
			else:
				print("Past data not found, please try again tomorrow.")
				sys.exit(0)
		query_data = pd.read_csv(query_csv, index_col = 0, compression = "gzip")
		try:
			query_data = query_data[query_data["id"] == args.query].values[0]
		except Exception:
			print("Studen id {} not found in {}\n".format(args.query, query_csv))
		query_data = query_data[-32:]
		query_name = pd.read_csv(os.path.join(data_path, args.course + "_info.csv.gz"), index_col = 0, compression = "gzip")
		query_name = query_name[query_name["student_id"] == args.query]["name"].values[0]

		query_delta = [0]
		for i in range(len(query_data) - 1):
			query_delta.append(query_data[i + 1] - query_data[i])
		print("{}'s last 31-day activity in {} listed:".format(query_name, args.course))
		for i in range(31, 0, -1):
			if (i >= len(query_delta)):
				continue
			print("{0: <10}\t{1}".format((datetime.date.today() - datetime.timedelta(i)).isoformat(), query_delta[-i]))
		contribution.contributionPlot(query_delta, by = "month", save = png_save_path, name = str(args.query))
		sys.exit(0)


	if (args.rank):
		if not (yesterday_cache_path):
			print("Past data of {} not found, please try again tomorrow.".format(args.course))
			sys.exit(0)
		yesterday_csv = os.path.join(yesterday_cache_path, args.course + ".csv.gz")
		delta = []
		df0 = pd.read_csv(yesterday_csv, index_col = 0, compression = "gzip")
		df1 = pd.read_csv(course_csv, index_col = 0, compression = "gzip")
		for i in range(len(df0)):
			row = df0.iloc[i]
			# print(row["total_activity_time"])
			activity_yesterday = row["total_activity_time"]
			activity_today = df1[df1["student_id"] == row["student_id"]]["total_activity_time"].values[0]
			delta_activity = activity_today - activity_yesterday
			delta.append(delta_activity)
		dfN = df0[["student_id", "name"]]
		dfN["day_activity"] = delta
		dfNsorted = dfN.groupby(["student_id", "name"]).agg(sum).sort_values("day_activity", ascending = False).head(15)
		print("-"*50 + "\nToday's 15 best contributors of " + args.course + "!\n" + "-"*50 + "\n")
		print(dfNsorted)
		sys.exit(0)

	if (args.mine):
		df = pd.read_csv(course_csv, index_col = 0, compression = "gzip")
		sys.stdout.write("{}".format(df[df["student_id"] == args.mine]["total_activity_time"].values[0]))
		sys.exit(0)

	if (args.favorite):
		course_list = findMyCourse()
		# print(course_list)
		sys.stdout.write(str(course_list))
		sys.exit(0)
	# print("no")
	# print(args.summary)
	# Write course data into csv
	df = readUsers(args.course)#.to_csv(course_csv)
	df = df.rename(columns = {"id": "student_id"})
	enrollment_keys = list(df["enrollments"][1][0].keys())
	for key in enrollment_keys:
		df[key] = df.apply(lambda row: extractEnroll(row, key), axis = 1)
	df = df.drop("enrollments", axis = 1)
	df = df.rename(columns = {"id": "enrollment_id"})
	df.to_csv(course_csv, compression = "gzip")
	# dfN = df.sort_values("total_activity_time", ascending = False)
	# dfN = dfN[["student_id", "name", "total_activity_time"]]
	# print(df.head(20))
	# print(args.course)


if __name__ == "__main__":
    main()
