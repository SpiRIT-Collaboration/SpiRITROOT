/*
class STHitSortByTrackDirection 
{
  private:
    std::vector<STHit*> *hits;
    TVector3 centroid;
    TVector3 direction;

  public:
    STHitSortByTrackDirection(std::vector<STHit*> *a, TVector3 c, TVector3 d)
    : hits(a), centroid(c), direction(d) {}

    Bool_t operator() (Int_t i, Int_t j)
    { return (hits -> at(i) -> GetPosition() - centroid).Dot(direction) >
             (hits -> at(j) -> GetPosition() - centroid).Dot(direction); }
    //{ 
    //  Bool_t b = ( (hits -> at(i) -> GetPosition() - centroid).Dot(direction) >
    //               (hits -> at(j) -> GetPosition() - centroid).Dot(direction) );
    //  cout << b << endl;
    //  return b;
    //}
};
*/

void sorthit()
{
  STLinearTrackFitter* fitter = new STLinearTrackFitter();
  STLinearTrack* track = new STLinearTrack();

  int n = 5;
  for (int i=0; i<n; i++) {
    TVector3 pos(0,0,((Int_t) gRandom -> Integer(100))-50);
    STHit* hit = new STHit(i, pos, 1);
    track -> AddHit(hit);
  }

  fitter -> FitAndSetTrack(track);

  std::cout << std::endl;
  std::cout << "direction vector:" << std::endl;
  track -> GetDirection().Print();

  std::vector<STHit*> *pointer_array = track -> GetHitPointerArray();
  std::vector<Int_t>  *id_array      = track -> GetHitIDArray();

  ////////////////////////////////////////////////////////////////////////

  std::cout << std::endl;
  std::cout << "pointer array before sort:" << std::endl;
  for (int i=0; i<n; i++)
    std::cout << (pointer_array -> at(i)) -> GetZ() << std::endl;

  std::cout << std::endl;
  std::cout << "id array before sort:" << std::endl;
  for (int i=0; i<n; i++)
    std::cout << id_array -> at(i) << std::endl;

  //********************************************************************//

  fitter -> SortHits(track);
  //std::cout << std::endl;
  //STHitSortByTrackDirection sorting(track -> GetHitPointerArray(), 
  //                                  track -> GetDirection(), 
  //                                  track -> GetCentroid());
  //std::sort(id_array -> begin(), id_array -> end(), sorting);

  //********************************************************************//

  std::cout << std::endl;
  std::cout << "pointer array after sort:" << std::endl;
  for (int i=0; i<n; i++)
    std::cout << (pointer_array -> at(i)) -> GetZ() << std::endl;

  std::cout << std::endl;
  std::cout << "id array after sort:" << std::endl;
  for (int i=0; i<n; i++)
  {
    std::cout << id_array -> at(i) << " "
              << (pointer_array -> at(id_array -> at(i))) -> GetZ() << std::endl;
  }
}
