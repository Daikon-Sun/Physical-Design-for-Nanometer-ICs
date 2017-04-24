for i in {0..0}
do
  echo "ami49"
  time ./main 0.5 input_pa2/ami49.block input_pa2/ami49.nets result/testing.rpt
  python check.py ami49 result/testing.rpt
  echo "ami33"
  time ./main 0.5 input_pa2/ami33.block input_pa2/ami33.nets result/testing.rpt
  python check.py ami33 result/testing.rpt
  echo "hp"
  time ./main 0.5 input_pa2/hp.block input_pa2/hp.nets result/testing.rpt
  python check.py hp result/testing.rpt
  echo "apte"
  time ./main 0.5 input_pa2/apte.block input_pa2/apte.nets result/testing.rpt
  python check.py apte result/testing.rpt
  echo "xerox"
  time ./main 0.5 input_pa2/xerox.block input_pa2/xerox.nets result/testing.rpt
  python check.py xerox result/testing.rpt
done
