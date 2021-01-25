local timer

function locationUpdate()
   local alt = math.random(100)
   local lat = math.random(14)
   local lng = math.random(9)
   local velocity = math.random(86)
   local timestamp = math.random(100)
   --""alt"":%%,""lat"":%%,""lng"":%%,""velocity"":%%,""timestamp"":""%%""
   c8y:send('329,' .. c8y.ID .. ',' .. alt',' .. lat',' .. lng',' .. velocity',' .. timestamp)
end

function init()
   timer = c8y:addTimer(1 * 1000, 'locationUpdate')
   timer:start()
   return 0
end