import unittest
from smartagri.state_store import StateStore
class StateTests(unittest.TestCase):
 def test_warning(self):
  s=StateStore();s.serial['connected']=True;s.update_telemetry({'T':40,'H':50,'AIR':300,'LIGHT':400,'SOIL':400,'TMIN':18,'TMAX':32,'HMIN':40,'HMAX':80,'AIRMAX':600,'LMIN':300,'SMIN':350,'VALID':3,'PUMP':0,'FAN':0,'LAMP':0,'BEEP':0})
  self.assertEqual(s.warnings()[0]['code'],'temp_high')
 def test_history_bound(self):
  s=StateStore()
  for i in range(400):s.update_telemetry({'T':i})
  self.assertEqual(len(s.history),300)
if __name__=='__main__':unittest.main()
