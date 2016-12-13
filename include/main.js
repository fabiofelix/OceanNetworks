const PATH_STATIONS = "http://dmas.uvic.ca/api/stations";
const PATH_ARCHIVE  = "http://dmas.uvic.ca/api/archivefiles";

const TOKEN_1 = "d56dc3dd-05bb-46d1-bf23-44443830f8b5";
const TOKEN_2 = "3c6cc174-9b36-44b6-8dd5-e9a562c93ae2";
const TOKEN_3 = "a3079716-7d07-4d4a-bb5d-e521ad60b784";
const TOKEN_4 = "ac68a483-c394-4ebf-882a-06ce11b08dea";

  var color = ["rgb(0, 0, 0)", "rgb(0, 0, 0)", "rgb(0, 0, 0)", "rgb(0, 0, 0)", "rgb(0, 0, 0)", //40-50
               "rgb(0, 0, 18)", "rgb(0, 0, 36)", "rgb(0, 0, 55)", "rgb(0, 0, 73)", "rgb(0, 0, 91)", //50-60
               "rgb(0, 0, 110)", "rgb(0, 0, 128)", "rgb(0, 0, 147)", "rgb(0, 0, 163)", "rgb(0, 0, 205)", //60-70
               "rgb(0, 0, 236)", "rgb(0, 0, 252)", "rgb(0, 53, 255)", "rgb(0, 103, 255)", "rgb(0, 149, 255)", //70-80
               "rgb(0, 189, 255)", "rgb(0, 220, 255)", "rgb(0, 242, 255)", "rgb(0, 253, 255)", "rgb(0, 255, 253)", //80-90
               "rgb(0, 255, 242)", "rgb(0, 255, 220)", "rgb(0, 255, 189)", "rgb(0, 255, 149)", "rgb(0, 255, 103)", //90-100
               "rgb(0, 255, 53)", "rgb( 53, 255, 0)", "rgb(103, 255, 0)", "rgb(149, 255, 0)", "rgb(189, 255, 0)", //100-110
               "rgb(220, 255, 0)", "rgb(242, 255, 0)", "rgb(253, 255, 0)", "rgb(255, 253, 0)", "rgb(255, 242, 0)", //110-120
               "rgb(255, 220, 0)", "rgb(255, 189, 0)", "rgb(255, 149, 0)", "rgb(255, 103, 0)", "rgb(255, 53, 0)", //120-130
               "rgb(252, 0, 0)", "rgb(236, 0, 0)", "rgb(205, 0, 0)", "rgb(163, 0, 0)", "rgb(114, 0, 0)", //130-140
               "rgb(255, 255, 255)"   
              ];
  var amplitude = [];
  
  var table = 
    "<table id='table_files' class='display' width='100%' cellspacing='0'>" +
      "<thead> " +
        "<tr>" +
          "{title}" +
        "</tr> " +
      "</thead>" +
      "<tbody>" +
        "{rows}" +
      "</tbody>" +
    "</table>";  

$(document).ready(
  function()
  {
    $("#load").click(handle_click_OK);
    $("#list_file").click(handle_click_file_list);
    $("#listing_station").click(getStationList);       
    
    $("#file_list").fileinput({showUpload: false, showPreview: false, initialCaption: "ou Carregar do .JSON"}); 
    $.datetimepicker.setLocale("pt-BR");
    $(".dates").datetimepicker(
      {
        format: "d/m/Y H:i:s",
        formatTime:"H:i:s",
        formatDate:"d/m/Y",
        mask: true,
        value: new Date()
      }      
    );    
    
    $('#color_value').slider({
      formatter: function(value) 
      {
        var qt_values = 51, first = 40; 
        amplitude = Array.apply(null, {length: qt_values}).map(function(value, index) { return first + index * 2; });      
      
        $("#color_valueSlider").find(".slider-selection").css("background", color[amplitude.indexOf(parseInt(value))]);
        $("#color_valueSlider").find(".slider-handle").css("background", color[amplitude.indexOf(parseInt(value))]);
                
        return value + " dB";
      }
    });
    $('#percent_value').slider({
      formatter: function(value) 
      {
        return value + " %";
      }
    });        
    
    $("#check_download_file_list").change(
      function()
      {
        $("#file_list").prop( "disabled", $("#check_download_file_list").is(":checked"));
        $("#file_list").val("");
      }
    );
    
    $("#check_download_image").change(
      function()
      {
        $("#image_directory").prop("disabled", $("#check_download_image").is(":checked"));
        $("#image_directory").val("");
      }
    );
  }    
);

function getFormattedDate(_date, format = "Y-m-d H:i:s.000")
{
  var formatter = new DateFormatter({dateSettings: $.extend({}, "", "")});
  
  if(format == "Y-m-d H:i:s.000")
    return formatter.formatDate(_date, format).replace(" ", "T") + "Z";
  else
    return formatter.formatDate(_date, format);
}

function disable(value)
{
  $("#form_fields").prop( "disabled", value);
  $("#loading").css("visibility",  value ? "visible" : "hidden");
  
  if(value)
    $("#grid_files").html("");
  else
  {
    $("#file_list").prop("disabled", $("#check_download_file_list").is(":checked"));
    $("#image_directory").prop("disabled", $("#check_download_image").is(":checked"));  
    $("#msg").html("");
  }
}

function validate()
{
  var msg = "";
  
  if(!$("#check_download_file_list").is(":checked") && $("#file_list").val() == "")
    msg += "- Arquivo .JSON\n";
  if(!$("#check_download_image").is(":checked") && $("#image_directory").val() == "")
    msg += "- Diretório de imagens\n";  
  if(!$("#d0").datetimepicker("getValue") || !$("#dn").datetimepicker("getValue"))
    msg += "- O intervalo";    

  if(msg != "")
  {
    alert("Informe:\n" + msg);
    return false;
  }
  
return true;  
}

function handle_click_OK()
{
  if(validate())
  {
    disable(true);    
    
    if($("#check_download_station_list").is(":checked"))
      getStationList("list");
    if($("#check_download_file_list").is(":checked")) 
    {
      var d0 = getFormattedDate($("#d0").datetimepicker("getValue")), 
          dn = getFormattedDate($("#dn").datetimepicker("getValue")); 

      $.ajax( {
        type:"Get",
        url: PATH_ARCHIVE,      
        data: "method=getList&token=" + TOKEN_1 + "&station=" + $("#station").val() + "&deviceCategory=HYDROPHONE&dateFrom=" + d0 + "&dateTo=" + dn + "&returnOptions=all&responseType=jsonp",          
        contentType: "application/json; charset=utf-8",
        dataType: "jsonp",      
        success:function(data) {
          
          var fi = filter_items(data);
          send_server(fi); 
          disable(false);      
        },
        error: function(xhr, msg, e) {
          
            disable(false);
            alert(msg);
            
          }
      });    
    }
    else
    {
      $.getJSON($("#file_list").val(),
        function(data)
        {
          var fi = filter_items(data);
          send_server(fi);
          disable(false);
        }
      ).fail(function( jqxhr, textStatus, error ) {
        disable(false);    
        alert(error);
      });    
    }    
  }
}

function handle_click_file_list()
{
  $.ajax({
      type: "POST",
      url: "include/main.php",
      data: {action: "file_list"},
      async: false,
    }).done(
      function(_data, _status)
      {
        if(_status == "success")
        {
          var obj = eval('(' + _data + ')'), rows = "";
          
          for(var i = 0; i < obj.length; i++)
          {
            rows +=               
              "<tr>" +
                "<td><a href=\"" + obj[i][0] + "\">" + obj[i][1] + "</a></td>" +
              "</tr>";     
          }
          
          $("#grid_files").html(table.replace("{title}", "<th>Arquivos</th>").replace("{rows}", rows) );
          config_grid_file();          
        }
      }        
    ).fail(
      function (jqXHR, textStatus, errorThrown) 
      { 
        ERROR += jqXHR.responseText + "\n";
      }
  );    
}

function getStationList(type)
{
  $("#msg").html("Baixando lista de estações...");
  
  $.ajax( {
    type:"Get",
    url: PATH_STATIONS,      
    data: "method=getTree&token=" + TOKEN_1 + "&responseType=jsonp",
    contentType: "application/json; charset=utf-8",
    dataType: "jsonp",      
    success:function(data) 
    {
      if(type == "list")
        $("#station").html(traverseTree(data, "", type));        
      else
      {
        $("#grid_files").html(table.replace("{title}", "<th>Código da Estação</th><th>Nome</th><th>Categoria</th><th>Descrição</th> ").replace("{rows}", traverseTree(data, "")));
        disable(false);            
        config_grid_file();
      }
    },
    error: function(xhr, msg, e) 
    {
      disable(false);    
      alert(msg);
    }
  });   
}

function traverseTree(treeNodes, name, type) 
{
  var text = "";
  
  for (var i = 0; i < treeNodes.length; i++) 
  {
    if(treeNodes[i].deviceCategories.indexOf("HYDROPHONE") >= 0)
    {
      if(type == undefined)
        text += 
          "<tr>" +
            "<td>" +  (treeNodes[i].stationCode ? treeNodes[i].stationCode : "") + "</td>" +
            "<td>" +  name + "/" + treeNodes[i].name + "</td>" +
            "<td>" +  (treeNodes[i].deviceCategories ? treeNodes[i].deviceCategories.join(', ') : "") + "</td>" +      
            "<td>" +  (treeNodes[i].description ? treeNodes[i].description : "") + "</td>" +
          "<tr>";
      else if(treeNodes[i].stationCode)
        text += "<option value=" + treeNodes[i].stationCode + ">" + treeNodes[i].stationCode + "</option>";
    }
    
    name += name == "" ? "" : "/";
    name += treeNodes[i].name.replace("Ocean Networks Canada", "");
    text += traverseTree(treeNodes[i].els, name, type);
  }
  
  return text;
}

function config_grid_file()
{
  $("#table_files").DataTable(
    {
      "language": {
    //     "decimal":        "",
        "emptyTable":     "Tabela vazia",
        "info": "Mostrando _PAGE_ página de _PAGES_ página(s)",
        "infoEmpty":      "Nenhum item",
        "infoFiltered":   "(filtrado de _MAX_ registros )",
    //     "infoPostFix":    "",
    //     "thousands":      ",",
        "lengthMenu":     "Mostrando _MENU_ registros",
        "loadingRecords": "Carregando...",
        "processing":     "Processando...",
        "search":         "Pesquisar:",
        "zeroRecords":    "Nenhum registro encontrado",
        "paginate": {
            "first":      "Primeiro",
            "last":       "Último",
            "next":       "Próximo",
            "previous":   "Anterior"
        },
        "aria": {
            "sortAscending":  ": Ascendentente",
            "sortDescending": ": Descendente"
        }    
      }
    }  
  ).on("click", "tr", function () {
    $(this).toggleClass("selected");
  });
  
//   alert( table.rows('.selected').data().length +' row(s) selected' );  
}

// function load_grid_file(list_file)
// {
//   var table = 
//     "<table id='table_files' class='display' width='100%' cellspacing='0'>" +
//       "<thead> " +
//         "<tr>" +
//           "<th>Arquivo</th>" +
//           "<th>Início da gravação</th> " +
//           "<th>Final da gravação</th> " +
//           "<th>Data de Arquivamento</th> " +
//           "<th>Última Modificação</th> " +
//           "<th>Tamanho do arquivo (KB)</th> " +
//           "<th>Tamanho do arquivo descomprimido (KB)</th> " +
// //           "<th>Tipo de compressão</th> " +
//         "</tr> " +
//       "</thead>" +
//       "<tbody>" +
//         "{rows}" +
//       "</tbody>" +
//     "</table>";  
//   
//   var rows = "";
//   
//   for(var i = 0; i < list_file.length; i++)
//   {
//     var file = list_file[i];
//     var filesize = file.fileSize / 1024.0,
//         uncompressedfilesize = file.uncompressedFileSize / 1024.0;        
//    
//     rows += 
//       "<tr>" +
//         "<td>" + file.filename + "</td>" +
//         "<td>" + getFormattedDate(new Date(file.dateFrom), "d/m/Y H:i:s") + "</td>" +
//         "<td>" + getFormattedDate(new Date(file.dateTo), "d/m/Y H:i:s") + "</td>" +
//         "<td>" + getFormattedDate(new Date(file.archivedDate), "d/m/Y H:i:s") + "</td>" +
//         "<td>" + getFormattedDate(new Date(file.modifyDate), "d/m/Y H:i:s") + "</td>" +
//         "<td>" + filesize.toFixed(2) + "</td>" +
//         "<td>" + uncompressedfilesize.toFixed(2) + "</td>" +
// //         "<td>" + file.compression + "</td>" +        
//       "</tr>";
//   }
//   
//   $("#grid_files").html( table.replace("{rows}", rows) );
//   config_grid_file();
// }

function filter_items(items)
{
  var new_items = [];
  var images =  $.grep(items, 
          function(file, index)
          {
            return file.filename.lastIndexOf(".png") >= 0 && file.filename.lastIndexOf("small") == -1  && file.filename.lastIndexOf("thumb") == -1;
          }
        );
  var audios =  $.grep(items, 
          function(file, index)
          {
            return file.filename.lastIndexOf(".wav") >= 0 /*|| file.filename.lastIndexOf(".mp3") >= 0*/;
          }
        );
  
  for(var i = 0; i < images.length; i++)
  {
    var current_file_name = images[i].filename.substr(1,  images[i].filename.lastIndexOf(".png") - 1);
    
    if(current_file_name.lastIndexOf("-spect") >= 0)
      current_file_name = current_file_name.substr(1,  current_file_name.lastIndexOf("-spect") - 1);  
    
    filtered = $.grep(audios, 
        function(file, index)
        {
          return file.filename.lastIndexOf(current_file_name) >= 0;
        }
      );   
    
    if(filtered.length > 0)
    {
       obj = {};
       obj["image"] = images[i];
       obj["audio"] = filtered;
       
       new_items.push(obj);
    }  
  }  
  
  return new_items;
}

function send_server(items)
{
  var ERROR = "", 
      rows = "",
     _threshold = color[amplitude.indexOf(parseInt($("#color_value").val()))].replace("rgb(", "").replace(")", ""),  
      color_range = Array.apply(null, color.slice(4)).map(
        function(value, index) 
        { 
          return value.replace("rgb(", "").replace(")", ""); 
        }
      );     
   
  for(var i = 0; i < items.length; i++)
  {
    $("#msg").html("Baixando áudios... (" + (i + 1) + " de " + items.length + ") ");
    
    $.ajax({
        type: "POST",
        url: "include/main.php",
        data: {action: "process", token: TOKEN_1,  data: JSON.stringify(items[i]), threshold: _threshold, percent: $("#percent_value").val(), 
               range_color: color_range, image_directory: $("#image_directory").val(), check_download_audio: $("#check_download_audio").is(":checked")},
        async: false,
      }).done(
        function(_data, _status)
        {
          if(_status == "success" && _data != "")
          {
            array_data = eval('(' + _data + ')');
            rows +=               
              "<tr>" +
                "<td><a href=\"" + array_data[0] + "\">" + array_data[1] + "</a></td>" +
              "</tr>";     
          }
        }        
      ).fail(
        function (jqXHR, textStatus, errorThrown) 
        { 
          ERROR += jqXHR.responseText + "\n";
        }
    );  
  }

  $("#grid_files").html(table.replace("{title}", "<th>Arquivos gerados</th>").replace("{rows}", rows));
  config_grid_file();
  alert(ERROR);
}
