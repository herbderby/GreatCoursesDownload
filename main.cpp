#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

class ParseUrl {
 public:
  ParseUrl(const string& url) {
    regex fileAndParams(R"((.*)\?(.*))");
    smatch matches;
    string fileUrl;
    string parameters;
    //                                     file  parameters
    if(regex_search(url, matches, regex(R"((.*)\?(.*))"))
        && matches.size() == 3) {
      fileUrl = matches.str(1);
      parameters = matches.str(2);
    }
    //                                           #lectures  dir      media
    if (regex_search(fileUrl, matches, regex(R"(([0-9]{2})_([^_]+)\.(...)$)"))
        && matches.size() == 4) {
      lastLecture_ = matches.str(1);
      directoryName_ = matches.str(2);
      mediaType_ = matches.str(3);
    }

    userId_ = parseParameter("userid", parameters);
    courseId_ = parseParameter("courseid", parameters);
    orderId_ = parseParameter("orderid", parameters);
  }

  string MakeUrl(const string& lectureNum) {
    string answer =
        "http://download.eastbaymedia-drm.com.edgesuite.net/anon.eastbaymedia-drm/courses/";
    answer += courseId_ + "/" + mediaType_ + "/" + filename(lectureNum);
    answer += "?userid=" + userId_ + "&orderid=" + orderId_ + "&courseid=" +
        courseId_ + "&FName=" + fileBase(lectureNum);
    return answer;
  }

  string filename(const string& lectureNum) {
    return fileBase(lectureNum) + "." + mediaType_;
  }

  string seriesDirectory() {
    return directoryName_;
  }

  int lastLecture() {
    return stoi(lastLecture_);
  }

 private:
  string fileBase(const string& lectureNum) {
    return "TGC_" + courseId_ + "_Lect" + lectureNum + "_" + directoryName_;
  }

  static string parseParameter(const string& parameter, const string&parameters) {
    string value;
    regex paramRegex(parameter + R"(=(.*?)&.*)");
    smatch matches;
    if (regex_search(parameters, matches, paramRegex) && matches.size() == 2) {
      value = matches.str(1);
    }
    return value;
  }

  string mediaType_;  // either mp3 or m4v
  string courseId_;
  string userId_;
  string orderId_;
  string directoryName_;
  string lastLecture_;
};

int main(int argc, char* argv[]) {
  cout << "Hello, World!" << endl;
  if (argc != 3) {
    cerr << "you need a destination directory and the url for the last "
        "lecture download." << endl;
    exit(1);
  }

  ParseUrl parseUrl(argv[2]);
  string directory = string(argv[1]) + "/" + parseUrl.seriesDirectory();

  int32_t exitValue;
  string mkdirCommand = "mkdir -p \"" + directory + "\"";
  exitValue = std::system(mkdirCommand.c_str());
  if (exitValue != 0) {
    cout << "Bad: " << exitValue << endl;
    exit (1);
  }

  for (int i = 1; i <= parseUrl.lastLecture(); ++i) {
    stringstream ss;
    ss << setw(2) << setfill('0') << i;
    string lectureNum = ss.str();
    cout << parseUrl.MakeUrl(lectureNum) << endl;
    string filename = directory + "/" + parseUrl.filename(lectureNum);
    string wgetCommand = "wget -O \"" + filename +
        "\" '" + parseUrl.MakeUrl(lectureNum) + "'";
    exitValue = std::system(wgetCommand.c_str());
    if (exitValue != 0) {
      cerr << "Bad: " << exitValue << endl;
      exit (1);
    }

  }

  return 0;
}