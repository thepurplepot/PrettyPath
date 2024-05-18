#include <curl/curl.h>
#include <fstream>
#include <iostream>

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}

// https://render.openstreetmap.org/cgi-bin/export?bbox=-3.54034423828125,54.29809495917476,-2.593460083007813,54.58857670681745&scale=140000&format=png
void download_map_tile(const std::string& url,
                       const std::string& output_filename) {
  CURL* curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Error: Failed to initialize libcurl" << std::endl;
    return;
  }

  FILE* fp = fopen(output_filename.c_str(), "wb");
  if (!fp) {
    std::cerr << "Error: Failed to open file " << output_filename << std::endl;
    curl_easy_cleanup(curl);
    return;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "Error: Failed to download " << url << ": "
              << curl_easy_strerror(res) << std::endl;
  }

  curl_easy_cleanup(curl);
  fclose(fp);
}

#include <opencv2/opencv.hpp>

int longitude_to_pixel(double longitude, double min_lon, double max_lon,
                       int image_width) {
  return (longitude - min_lon) / (max_lon - min_lon) * image_width;
}

int latitude_to_pixel(double latitude, double min_lat, double max_lat,
                      int image_height) {
  return (max_lat - latitude) / (max_lat - min_lat) * image_height;
}

void plot_path_on_map(const std::string& image_filename,
                      const std::vector<Node*>& path, double min_lat,
                      double max_lat, double min_lon, double max_lon) {
  // Load the image
  cv::Mat image = cv::imread(image_filename, cv::IMREAD_COLOR);
  if (image.empty()) {
    std::cerr << "Error: Failed to open image " << image_filename << std::endl;
    return;
  }

  // Convert the path to pixel coordinates
  std::vector<cv::Point> pixel_path;
  for (const Node* node : path) {
    int x = longitude_to_pixel(node->longitude, min_lon, max_lon, image.cols);
    int y = latitude_to_pixel(node->latitude, min_lat, max_lat, image.rows);
    pixel_path.push_back(cv::Point(x, y));
  }

  // Draw the path on the image
  for (size_t i = 0; i < pixel_path.size() - 1; ++i) {
    cv::line(image, pixel_path[i], pixel_path[i + 1], cv::Scalar(0, 0, 255), 2);
  }

  // Display the image
  cv::namedWindow("Map", cv::WINDOW_NORMAL);
  cv::imshow("Map", image);
  cv::waitKey(0);
}