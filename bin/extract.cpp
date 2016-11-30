/*
 * Normalização:
 * Haralick: MinMax [0, 1]
 *           OBS.: como o valor da entropia é negativo (cf. fórmula) ele sempre é o zero da normalização
 * LBP     : Probabilidade [0, 1]
 * Fourier : MinMax [0, 1]
 *           OBS.: nos testes o f0 sempre tem o valor mínimo (0) e o f11 sempre tem o valor máximo (1)
 * GCH     : Probabilidade [0, 1]
 * BIC     : Probabilidade [0, 1]
 * CCV     : Probabilidade [0, 1]
 * ACC     : Probabilidade [0, 1]
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <dirent.h>
#include <math.h>
#include <vector>
#include <stack>
#include <list>

#include <opencv2/opencv.hpp>
#include<float.h>
#include <complex>

#include <fstream>
#include <algorithm>
#include <iomanip>
#include <map>

// http://answers.opencv.org/question/85635/error-cannot-find-lippicv/

using namespace std;
using namespace cv;

//Processamento da lista de arquivos
unsigned char isFile   = 0x8;
unsigned char isFolder = 0x4;

//Utilizado para quantidação ao criar as matrizes de Haralick
const float max_value = 255;
//
const int length_matrix_haralick = 16;

//Criação do arquivo de características
ofstream csv_file;
bool init_header = false;

string int2str(int i)
{
  ostringstream os;
  os << i;

return os.str();
}

int str2int(const string& source)
{
  int value;
  istringstream buffer(source);
  buffer >> value;
  
return value;  
}

int quantization(int point)
{
  //Mapeia os valores de (0, max_value) para (0, length_matrix_haralick - 1)  
  return (int) round(point * (length_matrix_haralick - 1.0) / max_value);  
}

//direction = 0, 45, 90, 135
void create_coocurrence_matrix(Mat& img, Mat& matrix, int direction)
{
  int pixel1 = 0, pixel2 = 0;
  double total = 0.0;
  
  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    {
      pixel1 = quantization((int) img.ptr<uchar>(i)[j]);
      
      switch(direction)
      {
        case 0:
        {
          if(j < img.cols - 1)
            pixel2 = quantization((int) img.ptr<uchar>(i)[j + 1]);
          
          break;
        }
        case 45:
        {
          if(i > 0 && j < img.cols - 1)
            pixel2 = quantization((int) img.ptr<uchar>(i - 1)[j + 1]);
          
          break;    
        }
        case 90:
        {
          if(i > 0)
            pixel2 = quantization((int) img.ptr<uchar>(i - 1)[j]);
          
          break;    
        }
        case 135:
        {
          if(i > 0 && j > 0)
            pixel2 = quantization((int) img.ptr<uchar>(i - 1)[j - 1]);
          
          break;    
        }
        default:
          break;
      }
      
      total += 2; 
      matrix.ptr<double>(pixel1)[pixel2]++;   
      matrix.ptr<double>(pixel2)[pixel1]++;      
    }  
    
  for(int i = 0; i < length_matrix_haralick; i++)
    for(int j = 0; j < length_matrix_haralick; j++)
      matrix.ptr<double>(i)[j] = matrix.ptr<double>(i)[j] / total;
    
//   cout << "total_" << direction << ": " << total << "\n"
//        << matrix_0 << "\n";
}

double contrast(Mat &matrix_0, Mat &matrix_45, Mat &matrix_90, Mat &matrix_135)
{
  double valor_0 = 0.0, valor_45 = 0.0, valor_90 = 0.0, valor_135 = 0.0;
       
  for(int i = 0; i < length_matrix_haralick; i++)
    for(int j = 0; j < length_matrix_haralick; j++)
    {
      valor_0   += pow(i - j, 2) * matrix_0.ptr<double>(i)[j];
      valor_45  += pow(i - j, 2) * matrix_45.ptr<double>(i)[j];
      valor_90  += pow(i - j, 2) * matrix_90.ptr<double>(i)[j];
      valor_135 += pow(i - j, 2) * matrix_135.ptr<double>(i)[j];
    }
    
//   cout << "CONTRASTE MÉDIO\n"
//       << "valor_0   = " << valor_0 << "\n"  
//       << "valor_45  = " << valor_45 << "\n"
//       << "valor_90  = " << valor_90 << "\n"
//       << "valor_135 = " << valor_135 << "\n"    
//       << "média     = " << (valor_0 + valor_45 + valor_90 + valor_135) / 4 << "\n";  
      
  return (valor_0 + valor_45 + valor_90 + valor_135) / 4;
}

double angular_second_moment(Mat &matrix_0, Mat &matrix_45, Mat &matrix_90, Mat &matrix_135)
{
  double valor_0 = 0.0, valor_45 = 0.0, valor_90 = 0.0, valor_135 = 0.0;

  for(int i = 0; i < length_matrix_haralick; i++)
    for(int j = 0; j < length_matrix_haralick; j++)
    {
      valor_0   += pow(matrix_0.ptr<double>(i)[j], 2);
      valor_45  += pow(matrix_45.ptr<double>(i)[j], 2);
      valor_90  += pow(matrix_90.ptr<double>(i)[j], 2);
      valor_135 += pow(matrix_135.ptr<double>(i)[j], 2);
    } 
    
//   cout << "SEGUNDO MOMENTO ANGULAR\n"
//       << "valor_0   = " << valor_0 << "\n"  
//       << "valor_45  = " << valor_45 << "\n"
//       << "valor_90  = " << valor_90 << "\n"
//       << "valor_135 = " << valor_135 << "\n"    
//       << "média     = " << (valor_0 + valor_45 + valor_90 + valor_135) / 4 << "\n"; 
      
  return (valor_0 + valor_45 + valor_90 + valor_135) / 4;
}

double correlation_aux(Mat &matrix)
{
  double media_i = 0.0, media_j = 0.0, desvio_i = 0.0, desvio_j = 0.0, correlacao = 0.0;
  vector<double> Pi(length_matrix_haralick), Pj(length_matrix_haralick);
  
  for(int i = 0; i < length_matrix_haralick; i++)
  {  
    for(int j = 0; j < length_matrix_haralick; j++)
      Pi[i] += matrix.ptr<double>(i)[j];
    
    media_i += i * Pi[i];
  }   
  
  for(int j = 0; j < length_matrix_haralick; j++)
  {  
    for(int i = 0; i < length_matrix_haralick; i++)
      Pj[j] += matrix.ptr<double>(i)[j];
    
    media_j += j * Pj[j];
  }  
  
  for(int k = 0; k < length_matrix_haralick; k++)
  {  
    desvio_i += pow(k - media_i, 2) * Pi[k];
    desvio_j += pow(k - media_j, 2) * Pj[k];    
  } 
  
  desvio_i = sqrt(desvio_i);
  desvio_j = sqrt(desvio_j);  
  
  for(int i = 0; i < length_matrix_haralick; i++)
    for(int j = 0; j < length_matrix_haralick; j++)
      correlacao += ((i - media_i) * (j - media_j) * matrix.ptr<double>(i)[j]) / (desvio_i * desvio_j);
    
  return correlacao;
}

double correlation(Mat &matrix_0, Mat &matrix_45, Mat &matrix_90, Mat &matrix_135)
{
  double correlacao_0   = correlation_aux(matrix_0), 
         correlacao_45  = correlation_aux(matrix_45), 
         correlacao_90  = correlation_aux(matrix_90), 
         correlacao_135 = correlation_aux(matrix_135);
         
/*  cout << "CORRELAÇÃO\n"
      << "valor_0   = " << correlacao_0 << "\n"
      << "valor_45  = " << correlacao_45 << "\n"
      << "valor_90  = " << correlacao_90 << "\n"
      << "valor_135 = " << correlacao_135 << "\n"    
      << "média     = " << (correlacao_0 + correlacao_45 + correlacao_90 + correlacao_135) / 4 << "\n";  */ 
      
  return (correlacao_0 + correlacao_45 + correlacao_90 + correlacao_135) / 4;      
}

double entropy(Mat &matrix_0, Mat &matrix_45, Mat &matrix_90, Mat &matrix_135)
{
  double valor_0 = 0.0, valor_45 = 0.0, valor_90 = 0.0, valor_135 = 0.0;
 
  for(int i = 0; i < length_matrix_haralick; i++)
    for(int j = 0; j < length_matrix_haralick; j++)
    {
      valor_0   += matrix_0.ptr<double>(i)[j] * log2(matrix_0.ptr<double>(i)[j] + 1);
      valor_45  += matrix_45.ptr<double>(i)[j] * log2(matrix_45.ptr<double>(i)[j] + 1);
      valor_90  += matrix_90.ptr<double>(i)[j] * log2(matrix_90.ptr<double>(i)[j] + 1);
      valor_135 += matrix_135.ptr<double>(i)[j] * log2(matrix_135.ptr<double>(i)[j] + 1);
    } 
    
//   cout << "ENTROPIA\n"
//       << "valor_0   = " << valor_0 << "\n"  
//       << "valor_45  = " << valor_45 << "\n"
//       << "valor_90  = " << valor_90 << "\n"
//       << "valor_135 = " << valor_135 << "\n"    
//       << "média     = " << (-1) * (valor_0 + valor_45 + valor_90 + valor_135) / 4 << "\n";  
      
  return (-1) * (valor_0 + valor_45 + valor_90 + valor_135) / 4;
}

//Utilizado para caracterizar as texturas de uma imagem através das estatísticas de ocorrência dos nívies de cinza ao longo
//de diferentes direções
void extract_haralick(Mat& img, string &csv_header, vector<double> &values)
{
//--------------------MATRIZES DE CO-OCORRENCIA---------------------------------------------//  
  Mat matrix_0   = Mat::zeros(length_matrix_haralick, length_matrix_haralick, DataType<double>::type),
      matrix_45  = Mat::zeros(length_matrix_haralick, length_matrix_haralick, DataType<double>::type),
      matrix_90  = Mat::zeros(length_matrix_haralick, length_matrix_haralick, DataType<double>::type),
      matrix_135 = Mat::zeros(length_matrix_haralick, length_matrix_haralick, DataType<double>::type),       
      img_gray;
      
  cvtColor(img, img_gray, CV_BGR2GRAY);   
  
  create_coocurrence_matrix(img_gray, matrix_0, 0);
  create_coocurrence_matrix(img_gray, matrix_45, 45);
  create_coocurrence_matrix(img_gray, matrix_90, 90);
  create_coocurrence_matrix(img_gray, matrix_135, 135);  
  
//-----------------------------------------------------------------------------------------//  
  double hconstrast = contrast(matrix_0, matrix_45, matrix_90, matrix_135);
  double hangular_second_moment = angular_second_moment(matrix_0, matrix_45, matrix_90, matrix_135);
  double hcorrelation = correlation(matrix_0, matrix_45, matrix_90, matrix_135);
  double hentropy = entropy(matrix_0, matrix_45, matrix_90, matrix_135);
  
  double MIN = min(hconstrast, min(hangular_second_moment, min(hcorrelation, hentropy)));
  double MAX = max(hconstrast, max(hangular_second_moment, max(hcorrelation, hentropy)));  
  
  values.push_back((hconstrast - MIN) / (MAX - MIN));
  values.push_back((hangular_second_moment - MIN) / (MAX - MIN));
  values.push_back((hcorrelation - MIN) / (MAX - MIN));
  values.push_back((hentropy - MIN) / (MAX - MIN));

  if(csv_header.size() > 0)
    csv_header += ";";  
  
  csv_header += "\"hcontrast\";\"hangular_second_moment\";\"hcorrelation\";\"hentropy\"";
}

int minROR(int value)
{
  int code = value, min_code = value;
  int QT_POSITION = 1, QT_POSITION2 = 7;

  for(int i = 0; i < 8; i++)
  {
    if(code % 2 == 0)
      code = code >> QT_POSITION; //Desloca todo o número uma posição para a direita. ex.: 1100 1000 -> 0110 0100
    else
    {
      code = code >> QT_POSITION; //Desloca todo o número uma posição para a direita  ex.: 1100 1000 -> 0110 0100
      code |= 1 << QT_POSITION2;  //Coloca um 1 na primeira posição a esquerda        ex.: 0110 0000 | 1000 0000 -> 1110 0000 
    }
    
    if(code == value)
      break;
    
    min_code = min(min_code, code);
  }

  return min_code;
}

int uniform(int value)
{
  int count = 0, sum = 0, last = -1, end = -1, number = value;  

  if(number > 0)
  {
    while(true)
    {
      if(end == -1)
        end = number % 2;
      if(last != -1)
        sum += abs(number % 2 - last);
      if(number % 2 == 1)
        count++;
      
      last   = number % 2;
      number = number / 2;
      
      if(number <= 1)
        break;
    }  

    sum += abs(end - last);

    if(sum > 2)
      count = 9;
  }  

  return  count;    
}  

//Utilizado para caracterizar texturas de uma imagem
void extract_LBP(Mat &img, string &csv_header, vector<double> &values)
{
  int center = 0, code = 0;
  uchar *row = NULL;
  Mat matrix(img.rows, img.cols, DataType<uchar>::type), img_gray;
  
  cvtColor(img, img_gray, CV_BGR2GRAY);     
  
//---------------------------------LBP----------------------------------------------------------------------//  
  for(int i = 1 ; i < img_gray.rows - 1; i++)
  {
    row  = img_gray.ptr<uchar>(i);
   
    for(int j = 1; j < img_gray.cols - 1; j++)
    {
      code   = 0;
      center = row[j];

      code |= (img_gray.ptr<uchar>(i - 1)[j - 1] >= center) << 7; //Sendo verdade (true) 00000001 -> desloca o 1 sete posições   = 10000000
      code |= (img_gray.ptr<uchar>(i - 1)[j]     >= center) << 6; //Sendo verdade (true) 00000001 -> desloca o 1 seis posições   = 01000000
      code |= (img_gray.ptr<uchar>(i - 1)[j + 1] >= center) << 5; //Sendo verdade (true) 00000001 -> desloca o 1 cinco posições  = 00100000
      code |= (img_gray.ptr<uchar>(i)[j + 1]     >= center) << 4; //Sendo verdade (true) 00000001 -> desloca o 1 quatro posições = 00010000
      code |= (img_gray.ptr<uchar>(i + 1)[j + 1] >= center) << 3; //Sendo verdade (true) 00000001 -> desloca o 1 três posições   = 00001000
      code |= (img_gray.ptr<uchar>(i + 1)[j]     >= center) << 2; //Sendo verdade (true) 00000001 -> desloca o 1 duas posições   = 00000100
      code |= (img_gray.ptr<uchar>(i + 1)[j - 1] >= center) << 1; //Sendo verdade (true) 00000001 -> desloca o 1 uma posição     = 00000010
      code |= (img_gray.ptr<uchar>(i)[j - 1]     >= center) << 0; //Sendo verdade (true) 00000001 -> não desloca                 = 00000001                                                 
        
//       matrix.ptr<uchar>(i)[j] = (uchar) code;
//       matrix.ptr<uchar>(i)[j] = (uchar) minROR(code);
      matrix.ptr<uchar>(i)[j] = (uchar) uniform(code);
    }
  }
  
//---------------------------------HISTOGRAMA-------------------------------------------------------------------//  
  int qt_quant    = 9;  //16 quando não utilizar o uniform
  int qt_items[9] = {0};

  for(int i = 0; i < matrix.rows; i++)
    for(int j = 0; j < matrix.cols; j++)
    { 
      code = matrix.ptr<uchar>(i)[j]; 
      qt_items[code % qt_quant]++; 
    }

  double prob = 0.0;  
  
  //Calcula a probabilidade do pixel (divide por 256*256)
  for(int i = 0; i < qt_quant; i++)
  {
    prob = (double) qt_items[i] / (double) (matrix.rows * matrix.cols);

    if(csv_header.size() > 0)
      csv_header += ";";    
    
    csv_header += "\"lbp" + int2str(i + 1) + "\"";
    values.push_back(prob);
  }    
}

//informa de c1 deve vir ANTES de c2
bool mysort (complex<double> *c1, complex<double> *c2)
{
//   //decrescente
// //Altas frequências -> Baixas frequências
// //Detalhes finos    -> Forma
  return abs(*c1) > abs(*c2); 
  
//   //crescente
// //Baixas frequências -> Altas frequências
// //Forma              -> Detalhes finos
//   return abs(*c1) < abs(*c2); 
}    

void extract_fourier_border(Mat &img, string &csv_header, vector<double> &values)
{
  Mat img_gray;
  cvtColor(img, img_gray, CV_BGR2GRAY);       
  
//------------------------------ENCONTRANDO PONTOS DO CONTORNO DA IMAGEM----------------------------------------------//
  Mat target;
  vector< vector<Point> > contours;
  Canny(img_gray, target, 50.0, 150.0);  
  
  namedWindow("Original");  
  imshow("Original", img_gray);        
  
  namedWindow("Canny");  
  imshow("Canny", target);    
  
  findContours(target, contours, CV_RETR_LIST, CHAIN_APPROX_SIMPLE); 
//-------------------------------------------------------------------------------------------------------------------//

//--------------------------CONVERTENDO CONTORNO EM UMA MATRIX (1, X) DE COMPLEXOS-----------------------------------//
  int sum_column = 0, J = 0;

  for(int i = 0; i < contours.size(); i++)
    sum_column += (int) contours[i].size();

  Mat complex_contour(1, sum_column, DataType< complex<double> >::type);
  complex<double> c(0.0, 0.0);  
  
  for(int i = 0; i < contours.size(); i++)
  {
    for(int j = 0; j < contours[i].size(); j++)
    {
      Point p(contours[i][j]);
      
      c.real(p.x);
      c.imag(p.y);
      
      complex_contour.ptr< complex<double> >(0)[J] = c;
      J++;
    }
  }
//-------------------------------------------------------------------------------------------------------------------//

//-----------------------------------------DFT DO CONTORNO-----------------------------------------------------------//
  
  Mat descriptors;
  dft(complex_contour, descriptors);
 
//----------------------------------DESCONSIDERAR PARTE DAS COMPOENTES-----------------------------------------------//  
  vector< complex<double> * > order; 
  
  for(int i = 0; i < descriptors.cols; i++)
    order.push_back(&descriptors.ptr< complex<double> >(0)[i]);

  sort(order.begin(), order.end(), mysort);

  int init_i = order.size() * 0.9;
  
  for(int i = init_i; i < order.size(); i++)  
  {
    complex<double> *a = order[i];
    (*a).real(0.0);
    (*a).imag(0.0);
  }
  
//--------------------------------------------------------------------------------------------------//
  int count = 1;
  double abs1 = 0.0;

  for(int i = 0; i < descriptors.cols; i++)
  {
    abs1 = abs(descriptors.ptr< complex<double> >(0)[i]);    
    
    if(abs1 > 0)
    {
      values.push_back(abs1);  
      
      if(csv_header.size() > 0)
        csv_header += ";";      
      
      csv_header += "\"f" + int2str(count) + "\"";
      count++;
    }
  }
//--------------------------------------------------------------------------------------------------//    

  Mat inverseTransform;
  dft(descriptors, inverseTransform, cv::DFT_INVERSE | cv::DFT_SCALE);
  
  Mat teste(img_gray.rows, img_gray.cols, CV_LOAD_IMAGE_GRAYSCALE, Scalar(255, 255, 255));      
  
  for(int i = 1; i < inverseTransform.cols; i++)
  {
    complex<double> p0 = inverseTransform.ptr< complex<double> >(0)[i - 1];
    complex<double> p1 = inverseTransform.ptr< complex<double> >(0)[i];
    
    int x0 = (int) round(p0.real()),
        y0 = (int) round(p0.imag()),
        x1 = (int) round(p1.real()),
        y1 = (int) round(p1.imag());
    
        
    if(abs(x0 - x1) <= 10 && abs(y0 - y1) <= 10)    
      line(teste, Point(x0, y0), Point(x1, y1), CV_RGB(0, 0, 0), 2);
  }   
     
//   namedWindow("teste");  
//   imshow("teste", teste);    
    
  
//   waitKey(0);  
}

Mat opencv_dft(Mat &img)
{
  Mat padded;                            //expand input image to optimal size
  copyMakeBorder(img, padded, 0, getOptimalDFTSize(img.rows ) - img.rows, 0, getOptimalDFTSize(img.cols) - img.cols, BORDER_CONSTANT, Scalar::all(0));  
  
  Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
  Mat complexI = padded;
  merge(planes, 2, complexI);         

  dft(complexI, complexI);            

  return complexI;
}

Mat opencv_shift(Mat &complex)
{
  Mat planes[] = {Mat_<float>(complex), Mat::zeros(complex.size(), CV_32F)};
  split(complex, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
  magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
  Mat magI = planes[0];

  magI += Scalar::all(1);                    // switch to logarithmic scale
  log(magI, magI);

  // crop the spectrum, if it has an odd number of rows or columns
  magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

  // rearrange the quadrants of Fourier image  so that the origin is at the image center
  int cx = magI.cols/2;
  int cy = magI.rows/2;

  Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
  Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
  Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
  Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

  Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
  q0.copyTo(tmp);
  q3.copyTo(q0);
  tmp.copyTo(q3);

  q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
  q2.copyTo(q1);
  tmp.copyTo(q2);

  normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
                                          // viewable image form (float between values 0 and 1).  
  
  return magI;
}

void extract_fourier(Mat &img, string &csv_header, vector<double> &values)
{
  Mat img_gray;
  
  cvtColor(img, img_gray, CV_BGR2GRAY);     
  
  Mat comp  = opencv_dft(img_gray);
  Mat shifited = opencv_shift(comp);
  Mat result, last_mask, sub_mask, mask(shifited.rows, shifited.cols, DataType<uchar>::type, Scalar(0, 0, 0));
  int inc = round(shifited.rows / 20.0); //divide em 10 partes a distância entre a borda e o centro da imagem 
  
  if(shifited.cols < shifited.rows)
    inc = round(shifited.cols / 20.0);
  
  Point center(round(shifited.cols / 2.0), round(shifited.rows / 2.0));
  
  int TOTAL_DESCRIPTORS = 11;
  vector<double> descriptors;
  
  for(int i = 1; i <= TOTAL_DESCRIPTORS; i++)
  {
    circle(mask, center, i * inc, Scalar(255, 255, 255), -1);
    mask.copyTo(sub_mask);
   
    if(i > 1)
      sub_mask = mask - last_mask;
    if(i == 11)
      sub_mask = 255 - mask;
    
    result.release();
    shifited.copyTo(result, sub_mask);
    mask.copyTo(last_mask);  
   
    Scalar S = sum(result);
    complex<double> c(S[0], S[1]);
    descriptors.push_back(abs(c));
  }  

  normalize(descriptors, descriptors, 0, 1, CV_MINMAX);
  
  for(int i = 0; i < TOTAL_DESCRIPTORS; i++)
  {  
    if(csv_header.size() > 0)
      csv_header += ";";    
    
    csv_header += "\"f" + int2str(i) + "\"";
    values.push_back(descriptors[i]);
//     cout << "i: " << i << " - " << descriptors[i] << "\n";
  }  

//   waitKey(0);
}

string int2bin(int i)
{
  string b;
  int number = i, rest = 0;    
  
  while(true)
  {
    if(number == 0 || number == 1)
    {
      b += int2str(number);
      break;
    }    

    rest   = number % 2;
    number = number / 2;
    
    b += int2str(rest);
  }
  
  b = string(b.rbegin(), b.rend());

  ostringstream ss;
  ss << setw(8) << setfill('0') << b;
  b = ss.str();  
    
  return b;
}

//Busca os dois bits (esquerda) mais significativos de cada canal (BGR)
//Concatena esses 6 bits formando um novo número entre 0-63
//Obs.: na prática menos que 64b perde muita informação e mais que isso não acrescenta muita coisa
int get_code(Vec3b intensity)
{
  uchar blue  = intensity.val[0], 
        green = intensity.val[1], 
        red   = intensity.val[2];

  int FIRST_2_DIGIT = 192,        
      code = ((int) blue & FIRST_2_DIGIT)  >> 6 |
             ((int) green & FIRST_2_DIGIT) >> 4 |
             ((int) red & FIRST_2_DIGIT)   >> 2;

  return code;  
}

Mat quantization(Mat &img)
{
  Mat quant(img.rows, img.cols, DataType<uchar>::type); 
  
  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
      quant.ptr<uchar>(i)[j] = get_code(img.at<Vec3b>(i, j));      

  return quant;
}

//Descritor de cor - Global Color Histogram
//Apenas criar um histograma da imagem quantizada
void extract_GCH(Mat &img, string &csv_header, vector<double> &values)
{
  int qt_items[64] = {0};
  int code = 0;  

  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    { 
      code = img.ptr<uchar>(i)[j]; 
      qt_items[code]++; 
    }

  double prob = 0.0;  
  
  //Calcula a probabilidade do pixel
  for(int i = 0; i < 64; i++)
  {
    prob = (double) qt_items[i] / (double) (img.rows * img.cols);
    
    if(csv_header.size() > 0)
      csv_header += ";";    
    
    csv_header += "\"gch" + int2str(i + 1) + "\"";
    values.push_back(prob);
  }    
}

bool test_neighbor(Mat& img, int i, int j)
{
  uchar center = img.ptr<uchar>(i)[j];
  
         //Linha anterior i - 1
  return (i > 0 && j > 0 && (int) img.ptr<uchar>(i - 1)[j - 1] == center) && 
         (i > 0 && (int) img.ptr<uchar>(i - 1)[j] == center) &&
         (i > 0 && j < img.cols - 1 && (int) img.ptr<uchar>(i - 1)[j + 1] == center) &&
         //Mesma linha i  
         (j > 0 && (int) img.ptr<uchar>(i)[j - 1] == center) &&
         (j < img.cols - 1 && (int) img.ptr<uchar>(i)[j + 1] == center) &&
         //Linha posterior i + 1  
         (i < img.rows - 1 && j > 0 && (int) img.ptr<uchar>(i + 1)[j - 1] == center) &&
         (i < img.rows - 1 && (int) img.ptr<uchar>(i + 1)[j] == center) &&
         (i < img.rows - 1 && j < img.cols - 1 && (int) img.ptr<uchar>(i + 1)[j + 1] == center);         
}

//Descritor de cor - Border/Interior pixel Classification
//Verifica os 8 vizinhos de cada ponto. Se todos os vizinhos forem iguais ao pixel
//então contabiliza esse pixel como sendo parte da imagem, do contrário é uma porda
//Concatenar os dois histogramas ou criar um histograma com 128 posições. De 0-63 (borda), de 64-127 (imagem)
void extract_BIC(Mat &img, string &csv_header, vector<double> &values)
{
  int qt_items[128] = {0};
  int code = 0, INC_POSITION = 64;  

  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    { 
      code = img.ptr<uchar>(i)[j]; 
      
      if(test_neighbor(img, i, j))
        qt_items[code + INC_POSITION]++; 
      else
        qt_items[code]++; 
    }

  double prob = 0.0;  
  
  //Calcula a probabilidade do pixel
  for(int i = 0; i < 128; i++)
  {
    prob = (double) qt_items[i] / (double) (img.rows * img.cols);
    
    if(csv_header.size() > 0)
      csv_header += ";";
    
    csv_header += "\"bic" + int2str(i + 1) + "\"";
    values.push_back(prob);
  }    
}

//Descritor de cor - Color Coherence Vector
//Calcula a quantidade de pixels de todos dos componentes conexos de uma mesma cor dentro da imagem
//Divide esse componentes entre coerentes (0-63) ou não coerente (64-127) com base em
//um limiar
void extract_CCV(Mat &img, string &csv_header, vector<double> &values)
{
  Mat img_blur, img_quant;
  blur(img, img_blur, Size(3, 3));
  
// //precisa definir um sigma
// GaussianBlur(img, img_blur, Size(3, 3), 1.5)  ;
  
  img_quant = quantization(img);
  
  stack<Point> dfs;
  list<Point> component;
  Mat visited(img_quant.rows, img_quant.cols, DataType<bool>::type, Scalar(false));
  Point current, neighbor;
  
  int pixel = -1, n = -1, k0 = -1, kn = -1, l0 = -1, ln = -1, THRESHOLD = 100, INC_POSITION = 64, qt_items[128] = {0};
  
//   Mat teste(img_quant.rows, img_quant.cols, DataType<uchar>::type);    
  
  for(int i = 0; i < img_quant.rows; i++)
  {
    for(int j = 0; j < img_quant.cols; j++)
    {
      current.x = i;
      current.y = j;
      dfs.push(current);
   
      //busca em profundidade (DFS) para encontrar o componente conexo dessa cor se ele ainda não existe
      while(!dfs.empty())
      {
        current = dfs.top();
        dfs.pop(); 
        pixel =  img_quant.ptr<uchar>(current.x)[current.y];     
        
        if(!visited.ptr<bool>(current.x)[current.y])
        {
          visited.ptr<bool>(current.x)[current.y] = true;  
          
          if(component.empty())
            component.push_back(current);
          else
            component.insert(component.begin(), current);          
          
          k0 = current.x > 0 ? current.x - 1 : current.x; 
          kn = current.x < img_quant.rows - 1 ? current.x + 1 : current.x;
          l0 = current.y > 0 ? current.y - 1 : current.y; 
          ln = current.y < img_quant.cols - 1 ? current.y + 1 : current.y; 
          
          for(int k = k0; k <= kn; k++)
          {
            for(int l = l0; l <= ln; l++)
            {
              n = img_quant.ptr<uchar>(k)[l];             
              
              if ((k != current.x || l != current.y) && n == pixel)
              {
                neighbor.x = k;
                neighbor.y = l;
                dfs.push(neighbor);
              }
            }
          }                      
        }
      }  
      
      if(!component.empty())
      {
        if(component.size() >= THRESHOLD)
          qt_items[pixel] += component.size();
        else
          qt_items[pixel + INC_POSITION] += component.size();          
        
        component.clear();
      }
      
/*      while(!component.empty())
      {
        current = component.front();
        component.pop_front();
        teste.ptr<uchar>(current.x)[current.y] = img_quant.ptr<uchar>(i)[j];        
      }   */   
    }
  }
  
  double prob = 0.0;  
  
// //   Calcula a probabilidade do pixel
  for(int i = 0; i < 128; i++)
  {
    prob = (double) qt_items[i] / (double) (img.rows * img.cols);
    
    if(csv_header.size() > 0)
      csv_header += ";";
    
    csv_header += "\"ccv" + int2str(i + 1) + "\"";
    values.push_back(prob);
  }  
  
//   namedWindow("teste");    
//   imshow("teste", teste); 
//   waitKey(0);
}

//Descritor - Auto Color Correlation
//
void extract_ACC(Mat &img, string &csv_header, vector<double> &values)
{
  int pixel = 0,
      distances[4] = {1, 3, 5, 7}, 
      aux[64] = {0};  //níveis de cor quantizados = 64
  double autocorrelogram[4 * 64] = {0}; //quantidade de distâncias * níveis de cor
  
  for(int i = 0; i < 4; i++)
  {
    for(int j = distances[i]; j < img.rows - distances[i]; j++)
      for(int k = distances[i]; k < img.cols - distances[i]; k++)
        if(test_neighbor(img, j, k))
        {
          pixel = img.ptr<uchar>(j)[k];
          aux[pixel]++;
        }  
   
    int norm = (img.rows - distances[i]) * (img.cols - distances[i]);
    
    for(int l = 0; l < 64; l++)
      autocorrelogram[i * 64 + l] = (double) aux[l] / norm;
  }
  
  for(int i = 0; i < 4 * 64; i++)
  {
    if(csv_header.size() > 0)
      csv_header += ";";
    
    csv_header += "\"acc" + int2str(i + 1) + "\"";
    values.push_back(autocorrelogram[i]);    
  }
}

void extract_hist(Mat &img, string &csv_header, vector<double> &values)
{
  Mat quant(img.rows, img.cols, DataType<uchar>::type); 
  int qt_items[3] = {0};
  
  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    {  
      Vec3b intensity = img.at<Vec3b>(i, j);     
      uchar blue  = intensity.val[0], 
            green = intensity.val[1], 
            red   = intensity.val[2];  
            
      if((int) blue > 0)
        qt_items[0]++;
      if((int) green > 0)      
        qt_items[1]++;
      if((int) red > 0)      
        qt_items[2]++;
    }  
    
  double prob = 0.0;  

  //Calcula a probabilidade do pixel
  for(int i = 0; i < 3; i++)
  {
    prob = (double) qt_items[i] / (double) (img.rows * img.cols);
    
    if(csv_header.size() > 0)
      csv_header += ";";    
    
    csv_header += "\"hist" + int2str(i + 1) + "\"";
    values.push_back(prob);
  }
}

void process_image(string& path, int count)
{
  Mat img = imread(path.c_str());   
  
  if(img.data)
  {
    string csv_header = "";
    vector<double> values;    
    
//     cout << count << " -> Processando: " << path << "\n";
//     cout << "    Haralick\n";
//     extract_haralick(img, csv_header, values);
//     
//     cout << "    LBP\n";    
//     extract_LBP(img, csv_header, values);
//     
//     cout << "    Fourier\n";     
//     extract_fourier(img, csv_header, values);  

//     A quantização é realizada dentro do método
//     cout << "    CCV\n";        
//     extract_CCV(img, csv_header, values);   
    
//     cout << "    TESTE\n";            
      extract_hist(img, csv_header, values);
    
//     Mat img_quant = quantization(img);
//     cout << "    GCH\n";        
//     extract_GCH(img_quant, csv_header, values);
    
//     cout << "    BIC\n";        
//     extract_BIC(img_quant, csv_header, values);
    
//     cout << "    ACC\n";        
//     extract_ACC(img_quant, csv_header, values);
    
    if(init_header)
    {
      init_header = false;
      csv_file << csv_header << "\n";
    }
    
//     csv_file << path;
    
    for(int i = 0; i < values.size(); i++)
    {
      if(i > 0)
//         cout << ";";
        csv_file << ";";
      
//       cout << "\"" << values[i] << "\"";
      csv_file << "\"" << values[i] << "\"";
    }  
    
//     cout << "\n";
    csv_file << "\n";
  }  
}

bool iterate_directory(string path, string target)
{
//=======================UM ARQUIVO=================================//  
    csv_file.open(target.c_str(), ios::trunc);
    process_image(path, 1);
    csv_file.close();  
    return true;  
//=======================UM DIRETÓRIO=================================//      
//   DIR *dp = opendir(path.c_str());
// 
//   if (dp != NULL)
//   {
//     struct dirent *ep;
//     string path_aux;
// 
//     csv_file.open(target.c_str(), ios::trunc);
//     init_header = true;
//     int count = 0;
//     
//     process_image(path, count);    
//     
//     while (ep = readdir(dp))
//       if(strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0)
//       {
//         path_aux = path + "/" + ep->d_name;
// 
//         if(ep->d_type == isFolder)
//           iterate_directory(path_aux, target);
//         else
//         {
//           count++;
//           process_image(path_aux, count);
//         }  
//       }
//       
//     csv_file.close();  
//     closedir(dp);
//     
//     return true;
//   }
//   
//   return false;
}

int main(int argc, char* argv[])
{
  int msg = EXIT_SUCCESS;
//   
//   Mat img = imread(argv[1]);   
//   img = img(Rect(107, 66, 937, 745));
//   
//   string csv_header = "";
//   vector<double> values;      
//   
//   extract_hist(img, csv_header, values);
//   
//   for(int i = 0; i < 3; i++)
//   {
//     cout << values[i] << "\n";
//   }
  
  

//   namedWindow("Original");  
//   imshow("Original", img);        
// 
//   Mat img_quant = quantization(img);
//   
//   namedWindow("Quantizada");  
//   imshow("Quantizada", img_quant);   
//   
//   waitKey(0);
  
  
  if(argc == 3)
    iterate_directory(argv[1], argv[2]);
  else
  {
    cerr << "Informe os parâmetros corretamente: \n"
         << " [diretório com imagens]\n";
    msg = EXIT_FAILURE;         
  }         
   
  return msg;
}

