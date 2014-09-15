#pragma once

class STRiemannSort {
  public:
    enum ESortCriteria {
      kNoSort = -1,
      kSortX = 0,
      kSortY = 1,
      kSortZ = 2,
      kSortR = 3,
      kSortDistance = 4,
      kSortPhi = 5,
      kSortReversePhi = -5
    };
};
