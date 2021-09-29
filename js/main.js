const PATH_STATIONS = "http://dmas.uvic.ca/api/stations";
const PATH_ARCHIVE  = "http://dmas.uvic.ca/api/archivefiles";
const TOKEN_1 = "d56dc3dd-05bb-46d1-bf23-44443830f8b5";  //found in documentation

var OPTIONS = null;

class Table
{
  _this = this;

  constructor(wrapper)
  {
    this.wrapper = wrapper;
    this.count   = 0;
    this.create_table();
    this.viewer = new ViewData();
    this.viewer.join_audio_img = false;
    this.viewer.audio_path = "data/record/";    

    $(document).on("click", ".table_link", this, this.handle_click);
    $(document).on("mouseover", ".table_link", this, this.handle_mouseover);
    $(document).on("mouseleave", ".table_link", this, this.handle_mouseleave);    
  }

  handle_click(event)
  {
    event.preventDefault();

    event.data.viewer.show({
      name: $(this).html() + ".wav",
      img_suffix: "-spect"
    });
  }

  handle_mouseover(event)
  {
    event.data.viewer.show_image_tooltip(true, {name: $(this).html() + "-spect.png"}, false, {X: event.clientX, Y: event.clientY});
  }

  handle_mouseleave(event)
  {
    event.data.viewer.show_image_tooltip(false);    
  }

  create_table()
  {
    $(this.wrapper).html(
      "<table class='table'>" + 
      "<thead>" + 
        "<tr>" + 
          "<th scope='col'>#</th>" +
          "<th scope='col'>Description</th>" +
          "<th scope='col'>Dowloaded</th>" +
        "</tr>" +
      "</thead>" +
      "<tbody id='table_body'>" +
      "</tbody>" +
      "</table>");
  }

  add_row(file, view, id)
  {
    this.count++;
    $("#table_body").html(
      $("#table_body").html() + 
      "<tr id='table_row_" + this.count + "'>" + 
        "<th scope='row'>" + (id == undefined ? this.count : id) + "</th>" + 
        "<td>" + file + "</td>" + 
        "<td>" + view + "</td>" + 
      "</tr>");
  }

  change_last_row(file, view)
  {
    $("#table_row_" + this.count).html(
        "<th scope='row'>" + this.count + "</th>" + 
        "<td><a href='' class='table_link'>" + file + "</a></td>" + 
        "<td>" + view + "</td>"
    );
  }

  remove_last_row(dec = true)
  {
    $("#table_row_" + this.count).remove();

    if(dec)
      this.count--;
  }
};

$(document).ready(
  function()
  {
    OPTIONS = new HandleOption();    
    
    OPTIONS.config_options("#color", ".color_value", "#color_selected", "red", change_color);
    OPTIONS.config_options("#download", ".download_value", "#download_selected", "spectrogram");
    
    getStationList();

    var now = new Date();

    $('#from_date').val(now.format("yyyy-mm-dd"));
    $('#to_date').val(now.format("yyyy-mm-dd"));
    $("#color_threshold").on("input", threshold_input)
    $("#color_threshold").on("change", threshold_change)
    $("#search_button").on("click", submit_search)
  }
);

function getStationList()
{
  $("#process").html("Downloading STATION list...");

  $.ajax( {
    type:"Get",
    url: PATH_STATIONS,      
    data: "method=getTree&token=" + TOKEN_1 + "&responseType=jsonp",
    contentType: "application/json; charset=utf-8",
    dataType: "jsonp",      
    success:function(data) 
    {
      $("#process").html("");
      $("#station_list").html(traverseTree(data));      
      OPTIONS.config_options("#station", ".station_value", "#station_selected");
    },
    error: function(xhr, msg, e) 
    {
      alert(msg);
      $("#process").html("");
    }
  });   
}

function change_color(event)
{
  $("#color").removeClass("btn-danger");
  $("#color").removeClass("btn-success");
  $("#color").removeClass("btn-primary");
  $("#color").removeClass("btn-info");
  $("#color_threshold").prop("disabled", false); 

  switch($(event.target).attr("value"))
  {
    case "red":
    {
      $("#color").addClass("btn-danger");
      break;      
    }
    case "green":
    {
      $("#color").addClass("btn-success");
      break;      
    }
    case "blue":
    {
      $("#color").addClass("btn-info");
      break;      
    }
    default:
    {
      $("#color_threshold").val("0");
      $("#color_threshold").trigger("input");      
      $("#color_threshold").trigger("change");
      $("#color_threshold").prop("disabled", true);
      break;
    }  
  }
}

function traverseTree(treeNodes) 
{
  var text = "";
  
  for (var i = 0; i < treeNodes.length; i++) 
  {
    if(treeNodes[i].deviceCategories.indexOf("HYDROPHONE") >= 0 && treeNodes[i].stationCode)
    {
      text += "<li><a id='" + treeNodes[i].stationCode + "' class='station_value' value='" + treeNodes[i].stationCode + "'>" + treeNodes[i].stationCode + "</a></li>";
    }

    text += traverseTree(treeNodes[i].els);
  }
  
  return text;
}

function threshold_input()
{
  this.nextElementSibling.value = this.value;  
}

function threshold_change(event)
{
  console.log(this.value);
}

function validate()
{
  var msg = "";

  if ($("#station_selected").val() == "")
    msg += "Selection some station\n";
  if($("#from_date").val() == "" || $("#from_time").val() == "" )  
    msg += "Set from date\n";
  if($("#to_date").val() == "" || $("#to_time").val() == "" )  
    msg += "Set to date\n";    

  if(msg != "")
    alert(msg);

  return msg == "";
}

function submit_search(event)
{
  if(validate())
  {
    var d0 = new Date($("#from_date").val() + " " + $("#from_time").val()),
        df = new Date($("#to_date").val() + " " + $("#to_time").val());
    d0 = getFormattedDate(d0);    
    df = getFormattedDate(df, false);
    
    $("#process").html("Downloading FILE list...");

    $.ajax( {
      type:"Get",
      url: PATH_ARCHIVE,      
      data: "method=getList&token=" + TOKEN_1 + 
            "&station=" + $("#station_selected").val() + 
            "&deviceCategory=HYDROPHONE&dateFrom=" + d0 + "&dateTo=" + df + 
            "&returnOptions=all&responseType=jsonp",          
      contentType: "application/json; charset=utf-8",
      dataType: "jsonp",      
      success:function(data) 
      {
        $("#process").html("");  
        download_data(data);
      },
      error: function(xhr, msg, e) 
      {
        alert(msg);
      }
    });
  }
}

function getFormattedDate(_date, first = true)
{
  return _date.format("yyyy-mm-dd HH:MM:" +  (first ? "00" : "59")  + ".000").replace(" ", "T") + "Z"
}

function extract_info(data)
{
  var spec = [],
      record = [],
      merge = [];

  for(var i = 0; i < data.length; i++)
  {
    if(data[i].filename.includes("-spect.png"))
      spec.push(data[i].filename) 
    else if(data[i].filename.includes(".wav"))
      record.push(data[i].filename)       
  }

  spec.sort();
  record.sort();

  while(spec.length > 0 && record.length > 0)
  {
    spec_name   = spec[0].split(/\-spect.png/g).shift();
    record_name = record[0].split(/\.wav/g).shift();

    if(spec_name == record_name)
       merge.push({name: spec_name, spec:  spec.shift(), record: record.shift()});
    else
      spec.shift();
  }

  return merge;
}

function download_data(data)
{
   var ERROR = "";
   var download_obj = extract_info(data);
   var data = new Table("#process");   

   for(var i = 0; i < download_obj.length; i++)
   {
      data.add_row("<s>" + download_obj[i].name + "</s>", "", "Processing " + (i + 1) + "/" + download_obj.length);

      var aux = download_obj[i];
      aux["token"] = TOKEN_1;
      aux["color"] = $("#color_selected").val();
      aux["threshold"] = $("#color_threshold").val();
      aux["download"] = $("#download_selected").val();

      $.ajax({
        type: "POST",
        url: "srv/main.php",
        data: {action: "process", token: TOKEN_1,  data: JSON.stringify(aux)},
        async: false,
      }).done(
        function(_msg, _status)
        {
          if(_status == "success" && _msg != "")
          {
            if(_msg.toLowerCase().includes("spectrogram"))
              data.change_last_row(download_obj[i].name, "spec");
            else if(_msg.toLowerCase().includes("recording"))  
              data.change_last_row(download_obj[i].name, "spec+recording");
            else  
            {
              data.remove_last_row(false); 
              console.log(_msg);
            }  
          }
        }        
      ).fail(
        function (jqXHR, textStatus, errorThrown) 
        { 
          ERROR += jqXHR.responseText + "\n";
        }
      );  
   }
}
