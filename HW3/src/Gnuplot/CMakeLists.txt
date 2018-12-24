ADD_LIBRARY (Gnuplot
GnuplotLivePlotter.cpp GnuplotMatrixPlotter.cpp GnuplotPlotter.cpp
)
INCLUDE_DIRECTORIES (
        ${PROJECT_SOURCE_DIR}/Placement
        ${PROJECT_SOURCE_DIR}/PlaceCommon
        ${PROJECT_SOURCE_DIR}/
)
