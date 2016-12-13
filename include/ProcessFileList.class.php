<?php

  header("Content-Type:text/html; charset=ISO-8859-1", true);

class ProcessFileList extends Object
{
  public function execute()
  {
    $path_dir  = "../download/compact/";
    $files     = scandir($path_dir); 
    $rows_JSON = "";
    
    foreach($files as $item)
    {
      if($item != "." && $item != "..")
      { 
        $row  = array("download/compact/" . $item, $item);
        
        if($rows_JSON != "")
            $rows_JSON .= ", ";
            
        $rows_JSON .= json_encode($row);               
      }
    }    
    
    echo "[" . $rows_JSON . "]";
  }
}
?>
