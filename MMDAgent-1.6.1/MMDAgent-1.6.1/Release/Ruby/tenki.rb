require 'net/http'
require 'uri'
require 'json'

uri = URI.parse('http://weather.livedoor.com/forecast/webservice/json/v1?city=130010')
json = Net::HTTP.get(uri)
result = JSON.parse(json)
=begin
print result['title'], "\n"
print result['link'], "\n"
print "予報の発表日時:#{result['publicTime']}", "\n"
result['forecasts'].each do |forecast| 
  print "--------------------"
  print "予報日:#{forecast['dateLabel']}", "\n"
  print "天気:#{forecast['telop']}", "\n"
  print "最低気温:#{forecast['temperature']['min']}", "\n"
  print "最高気温:#{forecast['temperature']['max']}", "\n"
end
=end


begin
  FST_TEXT = <<"EOS"
# tenki_yoho_start
2 211 <eps> SYNTH_START|mei|mei_voice_normal|現在の東京の天気は#{forecast["telop"]}です。
#211 212 SYNTH_EVENT_STOP|mei SYNTH_START|mei|mei_voice_normal|#{ans}
211 0 SYNTH_EVENT_STOP|mei <eps>
# tenki_yoho_end
EOS

f = File.open("MMDAgent_Example.fst-tenki.fst")
content = f.read().gsub(/# tenki_yoho_start.*# tenki_yoho_end/m, FST_TEXT.tosjis)
File.open("MMDAgent_Example.fst-tenki.fst", 'w') { |file| file.write(content) }
rescue
  exit 1
end