<?php
  header("Content-Type:text/html; charset=ISO-8859-1", true);

class ProcessFile  extends Object
{
  private $url = "http://dmas.uvic.ca/api/archivefiles?method=getFile";
  private $image_directory = "../data/img/";
  private $record_directory = "../data/record/";
  private $token = "";
  private $filter_color = "";
  private $filter_threshold = 0;  
  private $perc_threshold = 0.5;

  private function evaluate_filter($img_name)
  {
    if($this->filter_color == "nofilter")
      return true;

    try
    {
      $spec = imagecreatefrompng($this->image_directory . $img_name);   
      $spec_cropped = imagecrop($spec, ['x' => 107, 'y' => 66, 'width' => 937, 'height' => 745]);
      $count_black = 0;

      for($y = 0; $y < imagesy($spec_cropped); $y++)
      {
        for($x = 0; $x < imagesx($spec_cropped); $x++)
        {
          $rgb = imagecolorat($spec_cropped, $x, $y);

          $r = ($rgb >> 16) & 0xFF;
          $g = ($rgb >> 8) & 0xFF;
          $b = $rgb & 0xFF;  
          $value = 0;

          if($this->filter_color == "red")
            $value = 2 * $r - $g - $b;
          else if($this->filter_color == "green")
            $value = 2 * $g - $r - $b;
          else
            $value = 2 * $b - $r - $g;

          if($value < $this->filter_threshold)
          {
            $count_black++;
            $black = imagecolorallocate($spec_cropped, 0, 0, 0);
            imagesetpixel($spec_cropped, $x, $y, $black);
          }  
        }
      }

      $count_total = imagesx($spec_cropped) * imagesy($spec_cropped);
      return ($count_total - $count_black) / $count_total >= $this->perc_threshold;

      // imagepng($spec_cropped, $this->image_directory . 'example-filter.png');
      // echo "x: " . imagesx($spec_cropped) . " - y: " . imagesy($spec_cropped);       
      // imagepng($spec_cropped, $this->image_directory . 'example-cropped.png');
    }
    finally
    {
      imagedestroy($spec_cropped);
      imagedestroy($spec);
    }
  }
  
  private function download_record($img_name, $record_name, $download)
  {
    if($this->evaluate_filter($img_name))
    {
      $record = file_get_contents($this->url . "&token=" . $this->token  . "&filename=" . $record_name); 

      if($record)
      {
        file_put_contents($this->record_directory . $record_name, $record);  
        return true;
      }  
      else
      {
        header("HTTP/1.1 500 Internal Server Error");
        echo "Recording appears to be an invalid recording source\n";    
      }             
    }

    return false;
  }

  public function execute()
  {
    $json_object     = json_decode(stripslashes($_REQUEST["data"]));

    $this->token            = $json_object->token; 
    $this->filter_color     = $json_object->color; 
    $this->filter_threshold = (int) $json_object->threshold;

    $img_name    = $json_object->spec;
    $record_name = $json_object->record;
    $download    = $json_object->download;

    $image = file_get_contents($this->url . "&token=" . $this->token  . "&filename=" . $img_name);
      
    if($image)
    {
      file_put_contents($this->image_directory . $img_name, $image); 

      if($download == "record")
      {
        if($this->download_record($img_name, $record_name, $download))
          echo "Recording (" . $record_name . ") downloaded\n";
        else  
        {
          unlink($this->image_directory . $img_name);
          echo $record_name . " was rejected by the filter\n";              
        }  
      }
      else
        echo "Spectrogram (" . $img_name . ") downloaded\n";      
    }  
    else
    {
      header("HTTP/1.1 500 Internal Server Error");
      echo "Image appears to be an invalid image source\n";    
    }     
  }  
}
?>
