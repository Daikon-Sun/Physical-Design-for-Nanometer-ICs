for i in {1..1}
do
  #echo "ami49"
  ./main $1 input_pa2/ami49.block input_pa2/ami49.nets result/testing.rpt --plot
  #python check.py ami49 result/testing.rpt $1
  #echo "ami33"
  ./main $1 input_pa2/ami33.block input_pa2/ami33.nets result/testing.rpt --plot
  #python check.py ami33 result/testing.rpt $1
  #echo "hp"
  ./main $1 input_pa2/hp.block input_pa2/hp.nets result/testing.rpt --plot
  #python check.py hp result/testing.rpt $1
  #echo "xerox"
  ./main $1 input_pa2/xerox.block input_pa2/xerox.nets result/testing.rpt --plot
  #python check.py xerox result/testing.rpt $1
  #echo "apte"
  ./main $1 input_pa2/apte.block input_pa2/apte.nets result/testing.rpt --plot
  #python check.py apte result/testing.rpt $1
  #echo "chen-hao 1"
  #time ./main $1 input_pa2/1.block input_pa2/1.nets result/testing.rpt
  #python check.py 1 result/testing.rpt $1
  #echo "chen-hao 2"
  #time ./main $1 input_pa2/2.block input_pa2/2.nets result/testing.rpt
  #python check.py 2 result/testing.rpt $1
  #echo "chen-hao 3"
  #time ./main $1 input_pa2/3.block input_pa2/3.nets result/testing.rpt
  #python check.py 3 result/testing.rpt $1
done
