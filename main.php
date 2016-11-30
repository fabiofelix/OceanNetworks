<?php
  header("Content-Type:text/html; charset=ISO-8859-1", true);
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: GET, POST');    
  
// Estação CBYIP
//1. Buscadas imagens do dia 23/03/2015 entre 00:00:00 e 23:59:59 (data de um dos testes do site). São 289 imagens .png
//2. Cortar a imagem para que fique apenas a região do espectrograma
//   107, 66, 937, 745
// 
// Estação CRIP
//1. Buscadas imagens do dia 05/10/2016 entre 00:00:00 e 23:59:59 (data de um dos testes do site).
// 
//OBS.: SE ESTIVER UTILIZANDO O APACHE, MODIFICAR O php.ini PARA QUE O SCRIPT POSSA DEMORAR MAIS NA SUA EXECUÇÃO
//  ;max_execution_time = 30 => 0
//  ;max_input_time = 60     => -1
  
  
//==================================FUNÇÕES PARA CÁCULO DE SIMILIRADIDADE==================================//    
  function euclidian($u, $v)
  {
    $dist = 0;
    
    if( count($u) == count($v) )
    {
      foreach($u as $key => $value)
      {
         $dist += pow($value - $v[$key], 2);         
      }   
         
      $dist = sqrt($dist);   
    }

    return $dist;
  }
  
  function manhatan($u, $v)
  {
    $dist = 0;
    
    if( count($u) == count($v) )
    {
      foreach($u as $key => $value)
      {
         $dist += abs($value - $v[$key]);         
      }   
         
      $dist = sqrt($dist);   
    }

    return $dist;
  }    
  
  function flog($value)
  {
    if($value == 0)
      return 0;
    else if(0 < $value && $value <= 1)
      return 1;
    else
      return ceil(log($value, 2)) + 1;
  }
  
  function dLog($u, $v)
  {
    $dist = 0;
    
    if( count($u) == count($v) )
    {
      
      foreach($u as $key => $value)
        $dist += abs(flog($value) - flog($v[$key]));
    }

    return $dist;    
  }
  
  function cosine($u, $v)
  {
    $dist = 0;
    
    if( count($u) == count($v) )
    {
      $s1 = 0;
      $s2 = 1;
      $s3 = 2;
      
      foreach($u as $key => $value)
      {
        $s1 += $value * $v[$key];
        $s2 += pow($value, 2);
        $s3 += pow($v[$key], 2);        
      }   
         
      $dist = $s1 / (sqrt($s2) * sqrt($s3));   
    }

    return $dist;
  }      
  
//=====================================================================================================//    
  
  require("wideimage/lib/WideImage.php");
  
  $base     = null;
  $tmp_conv = null;  
  
  function has_event($file_path)
  {
    $img = WideImage::load($file_path);
    $img = $img->crop(107, 66, 937, 745);
    $img->saveToFile('../download/current_img.png');  

    shell_exec("../bin/extract ../download/current_img.png ../download/current_img.csv");
    
    $handle2      = fopen("../download/current_img.csv", "r");
    $current_csv  = fgetcsv($handle2, 0, ";");
    $current_conv = array();
    
    foreach($current_csv as $value)
    {
      $current_conv[] = floatval($value);
    }    
    
    fclose($handle2);
    shell_exec("rm ../download/current_img.png ../download/current_img.csv");
    
    $BLUE  = 0;
    $GREEN = 1;
    $RED   = 2;
    
    $THRESHOLD_BLUE  = 0.330;
    $THRESHOLD_GREEN = 0.0418;
    $THRESHOLD_RED   = 0;

    return $current_conv[$RED] > $THRESHOLD_RED || ($current_conv[$GREEN] > $THRESHOLD_GREEN && $current_conv[$BLUE] > $THRESHOLD_BLUE);
  }   
  
//==================================LEITURA DE ARQUIVOS=======================================//  
   $json_object = json_decode(stripslashes($_REQUEST["data"]));
   $image_item  =  $json_object->image;
   $list_audio  = $json_object->audio;     
   
/*OBS.: NÃO ESQUECER DE ATRIBUIR AS PERMISSÕES DO DIRETÓRIO download PARA O USUÁRIO DO apache*/
  $image = file_get_contents("http://dmas.uvic.ca/api/archivefiles?method=getFile&token=" . $_REQUEST["token"]  . "&filename=" . $image_item->filename);
  file_put_contents("../download/img/" . $image_item->filename, $image);     

  echo "TESTE";
  
  if(has_event("../download/img/" . $image_item->filename))
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
    
    echo $path . "dowload/compact/" . $name. ".tar.bz2 "; 
  }
  else
  {
    shell_exec("rm ../download/img/" . $image_item->filename);      
  }  

?>
