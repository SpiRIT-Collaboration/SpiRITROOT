void mapTest() {
  STMapTest *test = new STMapTest();
  test -> SetAGETMap("../parameters/AGET.map");
  test -> SetUAMap("../parameters/UnitAsAd.map");

  test -> ShowAGETMap();
  test -> ShowUAMap();
}
