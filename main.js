const PATH_STATIONS = "http://dmas.uvic.ca/api/stations";
const PATH_ARCHIVE  = "http://dmas.uvic.ca/api/archivefiles";

const TOKEN_1 = "d56dc3dd-05bb-46d1-bf23-44443830f8b5";
const TOKEN_2 = "3c6cc174-9b36-44b6-8dd5-e9a562c93ae2";
const TOKEN_3 = "a3079716-7d07-4d4a-bb5d-e521ad60b784";
const TOKEN_4 = "ac68a483-c394-4ebf-882a-06ce11b08dea";

$(document).ready(
  function()
  {
    $("#load").click(handle_click_OK);
    
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
  $("#station").prop( "disabled", value);
  $("#d0").prop( "disabled", value);
  $("#dn").prop( "disabled", value);
  $("#load").prop( "disabled", value);  
  $("#loading").css("visibility",  value ? "visible" : "hidden");
  
  if(value)
    $("#grid_files").html("");
  else
    $("#msg").html("");
}

function handle_click_OK()
{
  
//   getStationList();
  disable(true);
  $("#msg").html("Buscando lista...");  
  
  $.getJSON("teste.json",
    function(data)
    {
      var fi = filter_items(data);
//       load_grid_file(fi);      
      send_server(fi);
//       disable(false);
    }
  ).fail(function( jqxhr, textStatus, error ) {
     disable(false);    
     alert(error);
  });
  
  
  
/*  $("#grid_files").html(""); 
  disable(true);
  $("#msg").html("Buscando lista...");  
  
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
  });*/  
}

function getStationList(type)
{
  $.ajax( {
    type:"Get",
    url: PATH_STATIONS,      
    data: "method=getTree&token=" + TOKEN_1 + "&responseType=jsonp",
    contentType: "application/json; charset=utf-8",
    dataType: "jsonp",      
    success:function(data) {
      
      if(type == "list")
        $("#station").html(traverseTree(data, "", type));        
      else
        $("#grid_files").html(    
          "<table id='table_files' class='display' width='100%' cellspacing='0'>" +
            "<thead> " +
              "<tr>" +
                "<th>Código da Estação</th>" +
                "<th>Nome</th> " +
                "<th>Categoria</th> " +
                "<th>Descrição</th> " +              
              "</tr> " +
            "</thead>" +
            "<tbody>" +
              traverseTree(data, "") +
            "</tbody>" +
          "</table>"
        );
    },
    error: function(xhr, msg, e) {
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
    
    text += traverseTree(treeNodes[i].els, name + "/" + treeNodes[i].name, type);
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

function load_grid_file(list_file)
{
  var table = 
    "<table id='table_files' class='display' width='100%' cellspacing='0'>" +
      "<thead> " +
        "<tr>" +
          "<th>Arquivo</th>" +
          "<th>Início da gravação</th> " +
          "<th>Final da gravação</th> " +
          "<th>Data de Arquivamento</th> " +
          "<th>Última Modificação</th> " +
          "<th>Tamanho do arquivo (KB)</th> " +
          "<th>Tamanho do arquivo descomprimido (KB)</th> " +
//           "<th>Tipo de compressão</th> " +
        "</tr> " +
      "</thead>" +
      "<tbody>" +
        "{rows}" +
      "</tbody>" +
    "</table>";  
  
  var rows = "";
  
  for(var i = 0; i < list_file.length; i++)
  {
    var file = list_file[i];
    var filesize = file.fileSize / 1024.0,
        uncompressedfilesize = file.uncompressedFileSize / 1024.0;        
   
    rows += 
      "<tr>" +
        "<td>" + file.filename + "</td>" +
        "<td>" + getFormattedDate(new Date(file.dateFrom), "d/m/Y H:i:s") + "</td>" +
        "<td>" + getFormattedDate(new Date(file.dateTo), "d/m/Y H:i:s") + "</td>" +
        "<td>" + getFormattedDate(new Date(file.archivedDate), "d/m/Y H:i:s") + "</td>" +
        "<td>" + getFormattedDate(new Date(file.modifyDate), "d/m/Y H:i:s") + "</td>" +
        "<td>" + filesize.toFixed(2) + "</td>" +
        "<td>" + uncompressedfilesize.toFixed(2) + "</td>" +
//         "<td>" + file.compression + "</td>" +        
      "</tr>";
  }
  
  $("#grid_files").html( table.replace("{rows}", rows) );
  config_grid_file();
}

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
  var table = 
    "<table id='table_files' class='display' width='100%' cellspacing='0'>" +
      "<thead> " +
        "<tr>" +
          "<th>Arquivo gerado</th>" +
        "</tr> " +
      "</thead>" +
      "<tbody>" +
        "{rows}" +
      "</tbody>" +
    "</table>";    
  
  var ERROR = "", rows = "";
  
  for(var i = 0; i < items.length; i++)
  {
    $("#msg").html("Baixando áudios... (" + (i + 1) + " de " + items.length + ") ");
    
    $.ajax({
        type: "POST",
        url: "include/main.php",
        data: {token: TOKEN_1,  data: JSON.stringify(items[i])},
        async: false,
      }).done(
        function(_data, _status)
        {
          if(_status == "success")
          {
            rows +=               
              "<tr>" +
                "<td>" + _data + "</td>" +
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

  disable(false);
  $("#grid_files").html( table.replace("{rows}", rows) );
  config_grid_file();  
}
