import unittest
from smartagri.protocol import parse_line,set_command,ctrl_command
class ProtocolTests(unittest.TestCase):
 def test_data(self):
  line='DATA,T=-2,H=55,AIR=300,LIGHT=500,SOIL=400,TMIN=-5,TMAX=32,HMIN=40,HMAX=80,AIRMAX=600,LMIN=300,SMIN=350,VALID=3,LED1=0,LED2=1,LED3=0,LED4=0,PUMP=0,FAN=0,LAMP=0,BEEP=0'
  d=parse_line(line);self.assertEqual(d['data']['T'],-2);self.assertEqual(d['data']['VALID'],3)
 def test_commands(self):
  self.assertTrue(set_command('1',{'TMIN':18,'TMAX':32,'HMIN':40,'HMAX':80,'AIRMAX':600,'LMIN':300,'SMIN':350}).startswith('SET,1,'))
  self.assertEqual(ctrl_command('2',{'pump':'on'},60),'CTRL,2,PUMP=ON,TTL=60\r\n')
 def test_bad(self):
  with self.assertRaises(ValueError):parse_line('DATA,T=1')
if __name__=='__main__':unittest.main()
