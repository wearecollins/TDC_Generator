alert(app.documents.length);
for ( var i=0; i<app.documents.length; i++){
  doIt(app.documents[i]);
}

function doIt( doc ){
  app.activeDocument = doc;
  var srcDoc = app.activeDocument;

  // call the current document
  var srcDoc = app.activeDocument;

  // set original width and height
  var imageW = srcDoc.width.value;
  var imageH = srcDoc.height.value;

  // get the info out of the source doc
  var fileName = srcDoc.name;
  var docName = fileName.substring(0,fileName.length -4);
  var filePath = srcDoc.path.toString();
  var fileExt = fileName.substring(fileName.length -4, fileName.length);

  var nameCheck = fileName.substring(0,fileName.indexOf("_bg"));
  if (fileName.indexOf("_bg") >=1)
  {
    var ind = fileName.indexOf("_bg");
     // no underscore so we need to open its namesake
     // alert(nameCheck)
     var filePair = filePath + "/../captures/" + docName.substring(0,ind) + ".png";//fileExt;

     openThisFile(filePair)
     activeDocument.selection.selectAll()
     activeDocument.selection.copy();
     app.activeDocument.close(SaveOptions.DONOTSAVECHANGES);
     app.activeDocument = srcDoc;
     activeDocument.paste()
     activeDocument.flatten();
     var newName = filePath + "/" + docName + "_done" + fileExt
     saveMe(newName)
  }
      else
      {
        app.activeDocument.close(SaveOptions.DONOTSAVECHANGES);
      }
}


function openThisFile(masterFileNameAndPath)
{
 var fileRef = new File(masterFileNameAndPath)
 if (fileRef.exists)
 //open that doc
 {
    app.open(fileRef);
 }
 else
 {
    alert("error opening " + masterFileNameAndPath)
 }
}

function saveMe(fPath)
{

// save out the image
var pngFile = new File(fPath);
pngSaveOptions = new PNGSaveOptions();
pngSaveOptions.embedColorProfile = true;
pngSaveOptions.formatOptions = FormatOptions.STANDARDBASELINE;
pngSaveOptions.matte = MatteType.NONE; pngSaveOptions.quality = 1;
activeDocument.saveAs(pngFile, pngSaveOptions, false, Extension.LOWERCASE);

// close that saved png
 app.activeDocument.close()
}