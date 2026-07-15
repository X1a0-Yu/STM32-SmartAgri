import json,unittest
from smartagri.flask_app import create_app
class FlaskTests(unittest.TestCase):
 def setUp(self):
  self.app=create_app(demo=True);self.client=self.app.test_client()
 def test_health(self):
  r=self.client.get('/api/v1/health');self.assertEqual(r.status_code,200);self.assertEqual(r.json['version'],'1.1.0')
 def test_system(self):
  r=self.client.get('/api/v1/system');self.assertEqual(r.json['protocol'],2);self.assertEqual(r.json['baud'],19200)
 def test_wireless(self):self.assertEqual(self.client.get('/api/v1/wireless').status_code,200)
 def test_threshold_validation(self):
  r=self.client.put('/api/v1/thresholds',json={'temp_min':33,'temp_max':30,'humi_min':40,'humi_max':80,'air_max':600,'light_min':300,'soil_min':350});self.assertEqual(r.status_code,409)
 def test_controls(self):self.assertEqual(self.client.put('/api/v1/controls/fan',json={'mode':'on','ttl_seconds':30}).status_code,200)
 def test_ingest(self):self.assertEqual(self.client.post('/api/v1/ingest/telemetry',json={'t':25}).status_code,200)
if __name__=='__main__':unittest.main()
