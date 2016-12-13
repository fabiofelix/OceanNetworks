<?php
  header("Content-Type:text/html; charset=ISO-8859-1", true);
  header('Access-Control-Allow-Origin: *');
  header('Access-Control-Allow-Methods: GET, POST');   
  
  require_once("Object.class.php");
  require_once("ProcessFile.class.php"); 
  require_once("ProcessFileList.class.php");
  
  $handle = null;

  switch ($_REQUEST["action"])
  {
    case "process":
    {
      $handle = new ProcessFile();
      break;
    }
    case "file_list":
    {
      $handle = new ProcessFileList();    
      break;
    }    
    default:
      break;
  }

  if($handle != null)
    $handle->execute();    
?>
