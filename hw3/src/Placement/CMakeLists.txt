ADD_LIBRARY (Placement
  Placement.cpp  Net.h Rectangle.h Module.h  Pin.h   Row.h Row.cpp
)
INCLUDE_DIRECTORIES (
        ${PROJECT_SOURCE_DIR}/Parser
        ${PROJECT_SOURCE_DIR}/Gnuplot
        ${PROJECT_SOURCE_DIR}
)
