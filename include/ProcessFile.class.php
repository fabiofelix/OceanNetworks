<?php
// Estação CBYIP
//1. Buscadas imagens do dia 23/03/2015 entre 00:00:00 e 23:59:59 (data de um dos testes do site). São 289 imagens .png
//2. Threshold: 70dB  e Percentual: 10%. Foram trazidos 16 arquivos
//3. Cortar a imagem para que fique apenas a região do espectrograma
//   107, 66, 937, 745
// 
// Estação CRIP
//1. Buscadas imagens do dia 01/10/2016 entre 00:00:00 e 23:59:59.  São 276 imagens .png
//2. Threshold: 80dB  e Percentual: 10% (TALVEZ SE BAIXAR PARA 5% TRAGA MAIS ARQUIVOS RELEVANTES). Foram trazidos 14 arquivos
// 
//OBS.: SE ESTIVER UTILIZANDO O APACHE, MODIFICAR O php.ini PARA QUE O SCRIPT POSSA DEMORAR MAIS NA SUA EXECUÇÃO
//  ;max_execution_time = 30 => 0
//  ;max_input_time = 60     => -1


  header("Content-Type:text/html; charset=ISO-8859-1", true);

class ProcessFile  extends Object
{
  private function has_event($file_path)
  {
    require("wideimage/lib/WideImage.php");
    $above_threshold = 0;
    
    if(file_exists("../bin/extract"))
    {
      $img = WideImage::load($file_path);
      $img = $img->crop(107, 66, 937, 745);
      $img->saveToFile('../download/current_img.png');
      
      if(file_exists("../download/current_img.png"))
      {
        $range_color = implode(";", $_REQUEST["range_color"]);
        $proportion = shell_exec("../bin/extract ../download/current_img.png \"" . $range_color . "\" \"" . $_REQUEST["threshold"] . "\"");
        $array_proportion = explode (";" , $proportion);
        $total = count($array_proportion);
        $above_threshold = 1.0 - floatval($array_proportion[$total - 1]);
      }
    }
    
    $percent = floatval($_REQUEST["percent"]) / 100.0;

    return $above_threshold > $percent;
  }

  public function execute()
  {
    $json_object = json_decode(stripslashes($_REQUEST["data"]));
    $image_directory = "../download/img/";
    $image_item  =  $json_object->image;
    $list_audio  = $json_object->audio;    
    
    if(empty($_REQUEST["image_directory"]))
    {
  /*OBS.: NÃO ESQUECER DE ATRIBUIR PARA O USUÁRIO DO apache TODAS AS PERMISSÕES DO DIRETÓRIO download */
      $image = file_get_contents("http://dmas.uvic.ca/api/archivefiles?method=getFile&token=" . $_REQUEST["token"]  . "&filename=" . $image_item->filename);
      
      if($image)
        file_put_contents($image_directory . $image_item->filename, $image);     
      else
      {
        header("HTTP/1.1 500 Internal Server Error");
        echo "Image appears to be an invalid image source\n";    
      } 
    }
    else
      $image_directory = $_REQUEST["image_directory"];
      
    $download_audio =  $_REQUEST["check_download_audio"] == "true" ? true : false;
    
    if($download_audio)
    {
      if($this->has_event($image_directory . $image_item->filename))
      {
        $name = pathinfo($image_item->filename, PATHINFO_FILENAME);
      
        if(strpos($image_item->filename, "-spect") !== false)
          $name = substr($name, 0, strpos($image_item->filename, "-spect"));

        foreach($list_audio as $f)
        {
          $audio = file_get_contents("http://dmas.uvic.ca/api/archivefiles?method=getFile&token=" . $_REQUEST["token"]  . "&filename=" . $f->filename);
          file_put_contents("../download/sound/" . $f->filename, $audio);    
          
          shell_exec(
            "cd ../download/sound/ && " .
            "cp ../img/" . $image_item->filename . " . && " .
            "tar -cjf ../compact/" . $name. ".tar.bz2 " . $name . "* && " .
            "rm *.png ../img/" . $image_item->filename . " ". $f->filename . " && ".
            "rm *.png ". $f->filename . " && ".        
            "cd ../../include "
            );
        }
        
        $path = getcwd();
        $path = substr($path, 0, strpos($path, "include"));
      
        echo "[\"download/compact/" . $name . ".tar.bz2\", \"" . $name . ".tar.bz2\"]"; 
      }
      else
        shell_exec("rm " . $image_directory . $image_item->filename);      
    }
    else
    {
      $target = "../download/img/";
    
      if(has_event($image_directory . $image_item->filename))  
        shell_exec("cp " . $image_directory . $image_item->filename . " " . $target);      
    }  
//==================================TESTES=======================================//  
//   $file_path = "../20161001/A/ICLISTENHF1350_20161001T010905.000Z-spect.png";
// 
//   $json_object = json_decode(stripslashes($_REQUEST["data"]));  
//   $range_color = implode(";", $_REQUEST["range_color"]);
//   
//   require("wideimage/lib/WideImage.php");
//   
//   $img = WideImage::load($file_path);
//   $img = $img->crop(107, 66, 937, 745);
//   $img->saveToFile('../download/current_img.png');
//   
//   $proportion = shell_exec("../bin/extract ../download/current_img.png \"" . $range_color . "\" \"" . $_REQUEST["threshold"] . "\"");
//   $array_proportion = explode (";" , $proportion);
//   $total = count($array_proportion);
//   
//   $above_threshold = 1.0 -  floatval($array_proportion[$total - 1]);
//   $percent = floatval($_REQUEST["percent"]) / 100;
//   
// //   var_dump($array_proportion);
//   if($above_threshold > $percent)
//     echo "OK";

//   echo $array_   proportion[$total - 1] . " -> " . $above_threshold;
  
//   $images = array_filter($json_object, function($obj)
//   {
//     return strpos($obj->image->filename, ".png") !== false;
//   });
//   
//   $audios = array_filter($json_object, function($obj)
//   {
//     return strpos($obj->image->filename, ".wav") !== false;
//   });  
// 
//   var_dump($audios);
//   
//   echo "\"" . implode(";", $_REQUEST["range_color"]) . "\"  \"" . $_REQUEST["threshold"] . "\" \n";

//   $path_dir = "../20161001/A/";
//   $files    = scandir($path_dir);
  
//   foreach($files as $item)
//   {
//     echo $item;
//     if($item != "." && $item != ".." && has_event($path_dir . $item))
//     { 
//         shell_exec("cp " . $path_dir . $item . " ../download/img/");    
          
//       $name = pathinfo($item, PATHINFO_FILENAME);
//       
//       if(strpos($item, "-spect") !== false)
//         $name = substr($name, 0, strpos($item, "-spect"));
//       
//       $filtered = array_filter($audios, function($obj) use ($name)
//       {
//         return strpos($obj->filename, $name) !== favar_dump(lse;
//       });
// 
//       foreach($filtered as $f)
//       {
//         $audio = file_get_contents("http://dmas.uvic.ca/api/archivefiles?method=getFile&token=" . $_REQUEST["token"]  . "&filename=" . $f->filename);
//         file_put_contents("../download/sound/" . $f->filename, $audio);    
//         
//         shell_exec(
//           "cd ../download/sound/ && " .
//           "cp ../../" .  $path_dir . $item. " . && " .
//           "tar -cjf ../compact/" . $name. ".tar.bz2 " . $name . "* && " .
//           "cd ../../include ");
//       }  
//     }
//   }
  }
  
// //==================================FUNÇÕES PARA CÁCULO DE SIMILIRADIDADE==================================//    
//   function euclidian($u, $v)
//   {
//     $dist = 0;
//     
//     if( count($u) == count($v) )
//     {
//       foreach($u as $key => $value)
//       {
//          $dist += pow($value - $v[$key], 2);         
//       }   
//          
//       $dist = sqrt($dist);   
//     }
// 
//     return $dist;
//   }
//   
//   function manhatan($u, $v)
//   {
//     $dist = 0;
//     
//     if( count($u) == count($v) )
//     {
//       foreach($u as $key => $value)
//       {
//          $dist += abs($value - $v[$key]);         
//       }   
//          
//       $dist = sqrt($dist);   
//     }
// 
//     return $dist;
//   }    
//   
//   function flog($value)
//   {
//     if($value == 0)
//       return 0;
//     else if(0 < $value && $value <= 1)
//       return 1;
//     else
//       return ceil(log($value, 2)) + 1;
//   }
//   
//   function dLog($u, $v)
//   {
//     $dist = 0;
//     
//     if( count($u) == count($v) )
//     {
//       
//       foreach($u as $key => $value)
//         $dist += abs(flog($value) - flog($v[$key]));
//     }
// 
//     return $dist;    
//   }
//   
//   function cosine($u, $v)
//   {
//     $dist = 0;
//     
//     if( count($u) == count($v) )
//     {
//       $s1 = 0;
//       $s2 = 1;
//       $s3 = 2;
//       
//       foreach($u as $key => $value)
//       {
//         $s1 += $value * $v[$key];
//         $s2 += pow($value, 2);
//         $s3 += pow($v[$key], 2);        
//       }   
//          
//       $dist = $s1 / (sqrt($s2) * sqrt($s3));   
//     }
// 
//     return $dist;
//   }      
//   
// //=====================================================================================================//    
  
}
?>