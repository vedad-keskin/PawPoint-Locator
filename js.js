
function Lokacija(){

    var Koordinate = document.getElementById("Koordinate");
    var koordinate = Koordinate.value;
    var GoogleSearch = "https://www.google.com/search?q=";
    

    var koordinateString = GoogleSearch + koordinate;

    window.open(koordinateString);
   }

      

  