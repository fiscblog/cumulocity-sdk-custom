local timer

function locationUpdate()
   local alt = 500
   latitudes = {46.94575870730012, 46.945528153617005, 46.94781710864971,
                46.9500378526418, 46.94976419966603, 46.9458000973342}
   longitudes = {7.453964912254063, 7.455323039921953, 7.4585906042915395,
                 7.457921237661333, 7.450879794887315, 7.447673367643552}
   local rand = math.random(5)
   local lat = latitudes[rand]
   local lng = longitudes[rand]
   local velocity = math.random(86)
   local timestamp = math.random(100)
   --""alt"":%%,""lat"":%%,""lng"":%%,""velocity"":%%,""timestamp"":""%%""
   c8y:send('329,' .. c8y.ID .. ',' .. alt .. ',' .. lat .. ',' .. lng .. ',' .. velocity .. ',' .. timestamp)
end

function init()
   timer = c8y:addTimer(5 * 1000, 'locationUpdate')
   timer:start()
   return 0
end
