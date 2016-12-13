#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <math.h>
#include <limits.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int str2int(const string& source)
{
  int value;
  istringstream buffer(source);
  buffer >> value;
  
return value;  
}

vector<Vec3b> string2vec(string value, Vec3b threshold = Vec3b(0, 0, 0))
{
  vector<Vec3b> return_vector;
  string token;
  
  int count = 0, R = -1, G = -1, B = -1;  
  
  for(int i = 0; i < value.size(); i++)
  {  
    if(value[i] != ' ')
    {  
      if(value[i] == ';' || i == value.size() - 1)
      {
        token.push_back(value[i]);
        B = str2int(token);
        Vec3b current(B, G, R);
        
        if(norm(current) > norm(threshold))
          return_vector.push_back(Vec3b(B, G, R));
        
        count = 0;
        token = "";
        R = -1;
        G = -1;
        B = -1;
      }
      else if(value[i] == ',')
      {
        count++;
        
        if(count == 1)
          R = str2int(token);
        else if(count == 2)
          G = str2int(token);
        else if(count == 3)
          B = str2int(token);
        
        token = "";
      }
      else 
        token.push_back(value[i]);
    }
  } 
  
return return_vector;
}

int _1NN(Vec3b value, vector<Vec3b> itens)
{
  int index = -1;
  double dist = DBL_MAX, dist_current = 0;
  
  for(int i = 0; i < itens.size(); i++)
  {
    Vec3b item =  itens[i]; 
    
    dist_current = sqrt(pow((int) item.val[0] - (int) value.val[0], 2) + pow((int) item.val[1] - (int) value.val[1], 2) +
      pow((int) item.val[2] - (int) value.val[2], 2));
    
    if(dist_current < dist)
    {
      index = i;
      dist  = dist_current;
    }
  }
  
  return index;
}

void extract_hist(Mat &img, vector<Vec3b> &range_color, Vec3b &threshold, vector<double> &values)
{
  int TOTAL = range_color.size() + 1,
      *qt_items = new int [TOTAL], index = -1;
    
  for(int i = 0; i < TOTAL; i++)
    qt_items[i] = 0;
  
  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    {  
      Vec3b intensity = img.at<Vec3b>(i, j);     
            
      if(norm(intensity) > norm(threshold))
      {
        index = _1NN(intensity, range_color);
        qt_items[index]++;
      }
      else
        qt_items[TOTAL - 1]++;
    }  
   
  double prob = 0, total_pixels = (double) (img.rows * img.cols);
  
  //Calcula a probabilidade do pixel
  for(int i = 0; i < TOTAL; i++)
  {
    prob = (double) qt_items[i] / total_pixels;
    values.push_back(prob);
//     cout << i << " -> " << prob << "\n"; 
  }      
}

int main(int argc, char* argv[])
{
  Mat img = imread(argv[1]);   
  string string_range(argv[2]), string_threshold(argv[3]);
  Vec3b threshold = string2vec(string_threshold)[0];
  vector<Vec3b> range_color = string2vec(string_range, threshold);
  vector<double> values;
  
//   medianBlur(img, img, 5);
  extract_hist(img, range_color, threshold, values);
  
  for(int i = 0; i < values.size(); i++)
  {
    if(i > 0)
      cout << ";";
    
    cout << values[i];
  }   
   
  return EXIT_SUCCESS;
}

