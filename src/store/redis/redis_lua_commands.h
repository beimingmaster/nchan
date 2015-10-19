//don't edit this please, it was auto-generated

typedef struct {
  //input: keys: [],  values: [ channel_id ]
  //output: channel_hash {ttl, time_last_seen, subscribers, messages} or nil
  // delete this channel and all its messages
  char *delete;

  //input: keys: [],  values: [ channel_id ]
  //output: channel_hash {ttl, time_last_seen, subscribers, messages} or nil
  // finds and return the info hash of a channel, or nil of channel not found
  char *find_channel;

  //input:  keys: [], values: [channel_id, msg_time, msg_tag, no_msgid_order, create_channel_ttl]
  //output: result_code, msg_time, msg_tag, message, content_type, channel_subscriber_count
  // no_msgid_order: 'FILO' for oldest message, 'FIFO' for most recent
  // create_channel_ttl - make new channel if it's absent, with ttl set to this. 0 to disable.
  // result_code can be: 200 - ok, 404 - not found, 410 - gone, 418 - not yet available
  char *get_message;

  //input:  keys: [], values: [channel_id, time, message, content_type, msg_ttl, max_msg_buf_size]
  //output: message_tag, channel_hash {ttl, time_last_seen, subscribers, messages}
  char *publish;

  //input:  keys: [], values: [channel_id, status_code]
  //output: current_subscribers
  char *publish_status;

  //input: keys: [], values: [channel_id, subscriber_id, channel_empty_ttl, active_ttl, concurrency]
  //  'subscriber_id' can be '-' for new id, or an existing id
  //  'active_ttl' is channel ttl with non-zero subscribers. -1 to persist, >0 ttl in sec
  //  'concurrency' can be 'FIFO', 'FILO', or 'broadcast'
  //output: subscriber_id, num_current_subscribers
  char *subscriber_register;

  //input: keys: [], values: [channel_id, subscriber_id, empty_ttl]
  // 'subscriber_id' is an existing id
  // 'empty_ttl' is channel ttl when without subscribers. 0 to delete immediately, -1 to persist, >0 ttl in sec
  //output: subscriber_id, num_current_subscribers
  char *subscriber_unregister;

} nhpm_redis_lua_scripts_t;

static nhpm_redis_lua_scripts_t nhpm_rds_lua_hashes = {
  "9af42e385bc489cae6453e569ed40423a52ab397",
  "c4a7535fa6d50cbdd4411d0c8cd2d2012c593f90",
  "288679b1de58e6754a2daccd3bba46244de6a629",
  "0facbb71d6594c6812533af0940c7d0d5c7ff007",
  "12ed3f03a385412690792c4544e4bbb393c2674f",
  "5657fcddff1bf91ec96053ba2d4ba31c88d0cc71",
  "255a859f9c67c3b7d6cb22f0a7e2141e1874ab48"
};

#define REDIS_LUA_HASH_LENGTH 40

static nhpm_redis_lua_scripts_t nhpm_rds_lua_script_names = {
  "delete",
  "find_channel",
  "get_message",
  "publish",
  "publish_status",
  "subscriber_register",
  "subscriber_unregister",
};

static nhpm_redis_lua_scripts_t nhpm_rds_lua_scripts = {
  //delete
  "--input: keys: [],  values: [ channel_id ]\n"
  "--output: channel_hash {ttl, time_last_seen, subscribers, messages} or nil\n"
  "-- delete this channel and all its messages\n"
  "local id = ARGV[1]\n"
  "local key_msg=    'channel:msg:%s:'..id --not finished yet\n"
  "local key_channel='channel:'..id\n"
  "local messages=   'channel:messages:'..id\n"
  "local subscribers='channel:subscribers:'..id\n"
  "local pubsub=     'channel:pubsub:'..id\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "if on then return function(...) \n"
  "local arg, cur = {...}, nil\n"
  "for i = 1, #arg do\n"
  "  arg[i]=tostring(arg[i])\n"
  "end\n"
  "redis.call('echo', table.concat(arg))\n"
  "  end; else\n"
  "    return function(...) return; end\n"
  "  end\n"
  "end)(enable_debug)\n"
  "\n"
  "dbg(' ####### DELETE #######')\n"
  "local num_messages = 0\n"
  "--delete all the messages right now mister!\n"
  "local msg\n"
  "while true do\n"
  "  msg = redis.call('LPOP', messages)\n"
  "  if msg then\n"
  "    num_messages = num_messages + 1\n"
  "    redis.call('DEL', key_msg:format(msg))\n"
  "  else\n"
  "    break\n"
  "  end\n"
  "end\n"
  "\n"
  "local del_msgpack =cmsgpack.pack({\"alert\", \"delete channel\", id})\n"
  "for k,channel_key in pairs(redis.call('SMEMBERS', subscribers)) do\n"
  "  redis.call('PUBLISH', channel_key, del_msgpack)\n"
  "end\n"
  "\n"
  "local nearly_departed = nil\n"
  "if redis.call('EXISTS', key_channel) ~= 0 then\n"
  "  nearly_departed = redis.call('hmget', key_channel, 'ttl', 'time_last_seen', 'subscribers')\n"
  "  for i = 1, #nearly_departed do\n"
  "    nearly_departed[i]=tonumber(nearly_departed[i]) or 0\n"
  "  end\n"
  "  \n"
  "  --leave some crumbs behind showing this channel was just deleted\n"
  "  redis.call('setex', \"channel:deleted:\"..id, 5, 1)\n"
  "  \n"
  "  table.insert(nearly_departed, num_messages)\n"
  "end\n"
  "\n"
  "redis.call('DEL', key_channel, messages, subscribers)\n"
  "\n"
  "if redis.call('PUBSUB','NUMSUB', pubsub)[2] > 0 then\n"
  "  redis.call('PUBLISH', pubsub, del_msgpack)\n"
  "end\n"
  "\n"
  "return nearly_departed",

  //find_channel
  "--input: keys: [],  values: [ channel_id ]\n"
  "--output: channel_hash {ttl, time_last_seen, subscribers, messages} or nil\n"
  "-- finds and return the info hash of a channel, or nil of channel not found\n"
  "local id = ARGV[1]\n"
  "local key_channel='channel:'..id\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "  if on then return function(...) redis.call('echo', table.concat({...})); end\n"
  "  else return function(...) return; end end\n"
  "end)(enable_debug)\n"
  "\n"
  "dbg(' #######  FIND_CHANNEL ######## ')\n"
  "\n"
  "if redis.call('EXISTS', key_channel) ~= 0 then\n"
  "  local ch = redis.call('hmget', key_channel, 'ttl', 'time_last_seen', 'subscribers')\n"
  "  for i = 1, #ch do\n"
  "    ch[i]=tonumber(ch[i]) or 0\n"
  "  end\n"
  "  table.insert(ch, redis.call('llen', \"channel:messages:\"..id))\n"
  "  return ch\n"
  "else\n"
  "  return nil\n"
  "end",

  //get_message
  "--input:  keys: [], values: [channel_id, msg_time, msg_tag, no_msgid_order, create_channel_ttl]\n"
  "--output: result_code, msg_time, msg_tag, message, content_type, channel_subscriber_count\n"
  "-- no_msgid_order: 'FILO' for oldest message, 'FIFO' for most recent\n"
  "-- create_channel_ttl - make new channel if it's absent, with ttl set to this. 0 to disable.\n"
  "-- result_code can be: 200 - ok, 404 - not found, 410 - gone, 418 - not yet available\n"
  "local id, time, tag, subscribe_if_current = ARGV[1], tonumber(ARGV[2]), tonumber(ARGV[3])\n"
  "local no_msgid_order=ARGV[4]\n"
  "local create_channel_ttl=tonumber(ARGV[5]) or 0\n"
  "local msg_id\n"
  "if time and time ~= 0 and tag then\n"
  "  msg_id=(\"%s:%s\"):format(time, tag)\n"
  "end\n"
  "\n"
  "-- This script has gotten big and ugly, but there are a few good reasons \n"
  "-- to keep it big and ugly. It needs to do a lot of stuff atomically, and \n"
  "-- redis doesn't do includes. It could be generated pre-insertion into redis, \n"
  "-- but then error messages become less useful, complicating debugging. If you \n"
  "-- have a solution to this, please help.\n"
  "\n"
  "local key={\n"
  "  next_message= 'channel:msg:%s:'..id, --hash\n"
  "  message=      'channel:msg:%s:%s', --hash\n"
  "  channel=      'channel:'..id, --hash\n"
  "  messages=     'channel:messages:'..id, --list\n"
  "--  pubsub=       'channel:subscribers:'..id, --set\n"
  "  subscriber_id='channel:next_subscriber_id:'..id, --integer\n"
  "\n"
  "}\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "  if on then return function(...) redis.call('echo', table.concat({...})); end\n"
  "  else return function(...) return; end end\n"
  "end)(enable_debug)\n"
  "\n"
  "dbg(' #######  GET_MESSAGE ######## ')\n"
  "\n"
  "local oldestmsg=function(list_key, old_fmt)\n"
  "  local old, oldkey\n"
  "  local n, del=0,0\n"
  "  while true do\n"
  "    n=n+1\n"
  "    old=redis.call('lindex', list_key, -1)\n"
  "    if old then\n"
  "      oldkey=old_fmt:format(old)\n"
  "      local ex=redis.call('exists', oldkey)\n"
  "      if ex==1 then\n"
  "        return oldkey\n"
  "      else\n"
  "        redis.call('rpop', list_key)\n"
  "        del=del+1\n"
  "      end \n"
  "    else\n"
  "      dbg(list_key, \" is empty\")\n"
  "      break\n"
  "    end\n"
  "  end\n"
  "end\n"
  "\n"
  "local tohash=function(arr)\n"
  "  if type(arr)~=\"table\" then\n"
  "    return nil\n"
  "  end\n"
  "  local h = {}\n"
  "  local k=nil\n"
  "  for i, v in ipairs(arr) do\n"
  "    if k == nil then\n"
  "      k=v\n"
  "    else\n"
  "      --dbg(k..\"=\"..v)\n"
  "      h[k]=v; k=nil\n"
  "    end\n"
  "  end\n"
  "  return h\n"
  "end\n"
  "\n"
  "if no_msgid_order ~= 'FIFO' then\n"
  "  no_msgid_order = 'FILO'\n"
  "end\n"
  "\n"
  "local channel = tohash(redis.call('HGETALL', key.channel))\n"
  "local new_channel = false\n"
  "if next(channel) == nil then\n"
  "  if create_channel_ttl==0 then\n"
  "    return {404, nil}\n"
  "  end\n"
  "  redis.call('HSET', key.channel, 'time', time)\n"
  "  redis.call('EXPIRE', key.channel, create_channel_ttl)\n"
  "  channel = {time=time}\n"
  "  new_channel = true\n"
  "end\n"
  "\n"
  "local subs_count = tonumber(channel.subscribers)\n"
  "\n"
  "local found_msg_id\n"
  "if msg_id==nil then\n"
  "  if new_channel then\n"
  "    dbg(\"new channel\")\n"
  "    return {418, \"\", \"\", \"\", \"\", subs_count}\n"
  "  else\n"
  "    dbg(\"no msg id given, ord=\"..no_msgid_order)\n"
  "    \n"
  "    if no_msgid_order == 'FIFO' then --most recent message\n"
  "      dbg(\"get most recent\")\n"
  "      found_msg_id=channel.current_message\n"
  "    elseif no_msgid_order == 'FILO' then --oldest message\n"
  "      dbg(\"get oldest\")\n"
  "      \n"
  "      found_msg_id=oldestmsg(key.messages, ('channel:msg:%s:'..id))\n"
  "    end\n"
  "    if found_msg_id == nil then\n"
  "      --we await a message\n"
  "      return {418, \"\", \"\", \"\", \"\", subs_count}\n"
  "    else\n"
  "      msg_id = found_msg_id\n"
  "      local msg=tohash(redis.call('HGETALL', msg_id))\n"
  "      if not next(msg) then --empty\n"
  "        return {404, \"\", \"\", \"\", \"\", subs_count}\n"
  "      else\n"
  "        dbg((\"found msg %s:%s  after %s:%s\"):format(tostring(msg.time), tostring(msg.tag), tostring(time), tostring(tag)))\n"
  "        return {200, tonumber(msg.time) or \"\", tonumber(msg.tag) or \"\", msg.data or \"\", msg.content_type or \"\", subs_count}\n"
  "      end\n"
  "    end\n"
  "  end\n"
  "else\n"
  "  if msg_id and channel.current_message == msg_id\n"
  "   or not channel.current_message then\n"
  "    return {418, \"\", \"\", \"\", \"\", subs_count}\n"
  "  end\n"
  "\n"
  "  key.message=key.message:format(msg_id, id)\n"
  "  local msg=tohash(redis.call('HGETALL', key.message))\n"
  "\n"
  "  if next(msg) == nil then -- no such message. it might've expired, or maybe it was never there\n"
  "    dbg(\"MESSAGE NOT FOUND\")\n"
  "    return {404, nil}\n"
  "  end\n"
  "\n"
  "  local next_msg, next_msgtime, next_msgtag\n"
  "  if not msg.next then --this should have been taken care of by the channel.current_message check\n"
  "    dbg(\"NEXT MESSAGE KEY NOT PRESENT. ERROR, ERROR!\")\n"
  "    return {404, nil}\n"
  "  else\n"
  "    dbg(\"NEXT MESSAGE KEY PRESENT: \" .. msg.next)\n"
  "    key.next_message=key.next_message:format(msg.next)\n"
  "    if redis.call('EXISTS', key.next_message)~=0 then\n"
  "      local ntime, ntag, ndata, ncontenttype=unpack(redis.call('HMGET', key.next_message, 'time', 'tag', 'data', 'content_type'))\n"
  "      dbg((\"found msg2 %i:%i  after %i:%i\"):format(ntime, ntag, time, tag))\n"
  "      return {200, tonumber(ntime) or \"\", tonumber(ntag) or \"\", ndata or \"\", ncontenttype or \"\", subs_count}\n"
  "    else\n"
  "      dbg(\"NEXT MESSAGE NOT FOUND\")\n"
  "      return {404, nil}\n"
  "    end\n"
  "  end\n"
  "end",

  //publish
  "--input:  keys: [], values: [channel_id, time, message, content_type, msg_ttl, max_msg_buf_size]\n"
  "--output: message_tag, channel_hash {ttl, time_last_seen, subscribers, messages}\n"
  "\n"
  "local id=ARGV[1]\n"
  "local time=tonumber(ARGV[2])\n"
  "local msg={\n"
  "  id=nil,\n"
  "  data= ARGV[3],\n"
  "  content_type=ARGV[4],\n"
  "  ttl= tonumber(ARGV[5]),\n"
  "  time= time,\n"
  "  tag= 0\n"
  "}\n"
  "local store_at_most_n_messages = ARGV[6]\n"
  "if store_at_most_n_messages == nil or store_at_most_n_messages == \"\" then\n"
  "  return {err=\"Argument 6, max_msg_buf_size, can't be empty\"}\n"
  "end\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "  if on then return function(...) \n"
  "    local arg, cur = {...}, nil\n"
  "    for i = 1, #arg do\n"
  "      arg[i]=tostring(arg[i])\n"
  "    end\n"
  "    redis.call('echo', table.concat(arg))\n"
  "  end; else\n"
  "    return function(...) return; end\n"
  "  end\n"
  "end)(enable_debug)\n"
  "\n"
  "if type(msg.content_type)=='string' and msg.content_type:find(':') then\n"
  "  return {err='Message content-type cannot contain \":\" character.'}\n"
  "end\n"
  "\n"
  "dbg(' #######  PUBLISH   ######## ')\n"
  "\n"
  "-- sets all fields for a hash from a dictionary\n"
  "local hmset = function (key, dict)\n"
  "  if next(dict) == nil then return nil end\n"
  "  local bulk = {}\n"
  "  for k, v in pairs(dict) do\n"
  "    table.insert(bulk, k)\n"
  "    table.insert(bulk, v)\n"
  "  end\n"
  "  return redis.call('HMSET', key, unpack(bulk))\n"
  "end\n"
  "\n"
  "local tohash=function(arr)\n"
  "  if type(arr)~=\"table\" then\n"
  "    return nil\n"
  "  end\n"
  "  local h = {}\n"
  "  local k=nil\n"
  "  for i, v in ipairs(arr) do\n"
  "    if k == nil then\n"
  "      k=v\n"
  "    else\n"
  "      h[k]=v; k=nil\n"
  "    end\n"
  "  end\n"
  "  return h\n"
  "end\n"
  "\n"
  "local key={\n"
  "  time_offset=  'nchan:message_time_offset',\n"
  "  last_message= nil,\n"
  "  message=      'channel:msg:%s:'..id, --not finished yet\n"
  "  channel=      'channel:'..id,\n"
  "  messages=     'channel:messages:'..id,\n"
  "  subscribers=  'channel:subscribers:'..id,\n"
  "  subscriber_id='channel:next_subscriber_id:'..id, --integer\n"
  "}\n"
  "\n"
  "local channel_pubsub = 'channel:pubsub:'..id\n"
  "\n"
  "local new_channel\n"
  "local channel\n"
  "if redis.call('EXISTS', key.channel) ~= 0 then\n"
  "  channel=tohash(redis.call('HGETALL', key.channel))\n"
  "end\n"
  "\n"
  "if channel~=nil then\n"
  "  dbg(\"channel present\")\n"
  "  if channel.current_message ~= nil then\n"
  "    dbg(\"channel current_message present\")\n"
  "    key.last_message=('channel:msg:%s:%s'):format(channel.current_message, id)\n"
  "  else\n"
  "    dbg(\"channel current_message absent\")\n"
  "    key.last_message=nil\n"
  "  end\n"
  "  new_channel=false\n"
  "else\n"
  "  dbg(\"channel missing\")\n"
  "  channel={}\n"
  "  new_channel=true\n"
  "  key.last_message=nil\n"
  "end\n"
  "\n"
  "--set new message id\n"
  "if key.last_message then\n"
  "  local lastmsg = redis.call('HMGET', key.last_message, 'time', 'tag')\n"
  "  local lasttime, lasttag = tonumber(lastmsg[1]), tonumber(lastmsg[2])\n"
  "  dbg(\"New message id: last_time \", lasttime, \" last_tag \", lasttag, \" msg_time \", msg.time)\n"
  "  if lasttime==msg.time then\n"
  "    msg.tag=lasttag+1\n"
  "  end\n"
  "end\n"
  "msg.id=('%i:%i'):format(msg.time, msg.tag)\n"
  "\n"
  "key.message=key.message:format(msg.id)\n"
  "if redis.call('exists', key.message) ~= 0 then\n"
  "  return {err=(\"Message for channel %s id %s already exists\"):format(id, msg.id)}\n"
  "end\n"
  "\n"
  "msg.prev=channel.current_message\n"
  "if key.last_message then\n"
  "  redis.call('HSET', key.last_message, 'next', msg.id)\n"
  "end\n"
  "\n"
  "--update channel\n"
  "redis.call('HSET', key.channel, 'current_message', msg.id)\n"
  "if msg.prev then\n"
  "  redis.call('HSET', key.channel, 'prev_message', msg.prev)\n"
  "end\n"
  "if msg.time then\n"
  "  redis.call('HSET', key.channel, 'time', msg.time)\n"
  "end\n"
  "if not channel.ttl then\n"
  "  channel.ttl=msg.ttl\n"
  "  redis.call('HSET', key.channel, 'ttl', channel.ttl)\n"
  "end\n"
  "\n"
  "if not channel.max_stored_messages then\n"
  "  channel.max_stored_messages = store_at_most_n_messages\n"
  "  redis.call('HSET', key.channel, 'max_stored_messages', store_at_most_n_messages)\n"
  "  dbg(\"channel.max_stored_messages was not set, but is now \", store_at_most_n_messages)\n"
  "else\n"
  "  channel.max_stored_messages =tonumber(channel.max_stored_messages)\n"
  "  dbg(\"channel.mas_stored_messages == \" , channel.max_stored_messages)\n"
  "end\n"
  "\n"
  "--write message\n"
  "hmset(key.message, msg)\n"
  "\n"
  "\n"
  "--check old entries\n"
  "local oldestmsg=function(list_key, old_fmt)\n"
  "  local old, oldkey\n"
  "  local n, del=0,0\n"
  "  while true do\n"
  "    n=n+1\n"
  "    old=redis.call('lindex', list_key, -1)\n"
  "    if old then\n"
  "      oldkey=old_fmt:format(old)\n"
  "      local ex=redis.call('exists', oldkey)\n"
  "      if ex==1 then\n"
  "        return oldkey\n"
  "      else\n"
  "        redis.call('rpop', list_key)\n"
  "        del=del+1\n"
  "      end \n"
  "    else\n"
  "      break\n"
  "    end\n"
  "  end\n"
  "end\n"
  "\n"
  "local max_stored_msgs = tonumber(redis.call('HGET', key.channel, 'max_stored_messages')) or -1\n"
  "\n"
  "if max_stored_msgs < 0 then --no limit\n"
  "  oldestmsg(key.messages, 'channel:msg:%s:'..id)\n"
  "  redis.call('LPUSH', key.messages, msg.id)\n"
  "elseif max_stored_msgs > 0 then\n"
  "  local stored_messages = tonumber(redis.call('LLEN', key.messages))\n"
  "  redis.call('LPUSH', key.messages, msg.id)\n"
  "  if stored_messages > max_stored_msgs then\n"
  "    local oldmsgid = redis.call('RPOP', key.messages)\n"
  "    redis.call('DEL', 'channel:msg:'..id..':'..oldmsgid)\n"
  "  end\n"
  "  oldestmsg(key.messages, 'channel:msg:%s:'..id)\n"
  "end\n"
  "\n"
  "\n"
  "--set expiration times for all the things\n"
  "for i, k in pairs(key) do\n"
  "  if i ~= 'last_message' then\n"
  "    redis.call('EXPIRE', k, channel.ttl)\n"
  "  end\n"
  "end\n"
  "\n"
  "--publish message\n"
  "local unpacked\n"
  "\n"
  "if #msg.data < 5*1024 then\n"
  "  unpacked= {\n"
  "    \"msg\",\n"
  "    msg.time,\n"
  "    tonumber(msg.tag),\n"
  "    msg.data,\n"
  "    msg.content_type\n"
  "  }\n"
  "else\n"
  "  unpacked= {\n"
  "    \"msgkey\",\n"
  "    msg.time,\n"
  "    tonumber(msg.tag),\n"
  "    key.message\n"
  "  }\n"
  "end\n"
  "\n"
  "local msgpacked = cmsgpack.pack(unpacked)\n"
  "\n"
  "dbg((\"Stored message with id %i:%i => %s\"):format(msg.time, msg.tag, msg.data))\n"
  "\n"
  "--now publish to the efficient channel\n"
  "if redis.call('PUBSUB','NUMSUB', channel_pubsub)[2] > 0 then\n"
  "  msgpacked = cmsgpack.pack(unpacked)\n"
  "  redis.call('PUBLISH', channel_pubsub, msgpacked)\n"
  "end\n"
  "\n"
  "local num_messages = redis.call('llen', key.messages)\n"
  "\n"
  "dbg(\"channel \", id, \" ttl: \",channel.ttl, \", subscribers: \", channel.subscribers, \", messages: \", num_messages)\n"
  "return { msg.tag, {tonumber(channel.ttl or msg.ttl), tonumber(channel.time or msg.time), tonumber(channel.subscribers or 0), tonumber(num_messages)}, new_channel}",

  //publish_status
  "--input:  keys: [], values: [channel_id, status_code]\n"
  "--output: current_subscribers\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "if on then\n"
  "  return function(...)\n"
  "  redis.call('echo', table.concat({...}))\n"
  "end\n"
  "  else\n"
  "    return function(...)\n"
  "    return\n"
  "    end\n"
  "  end\n"
  "end)(enable_debug)\n"
  "dbg(' ####### PUBLISH STATUS ####### ')\n"
  "local id=ARGV[1]\n"
  "local code=tonumber(ARGV[2])\n"
  "if code==nil then\n"
  "  return {err=\"non-numeric status code given, bailing!\"}\n"
  "end\n"
  "\n"
  "local pubmsg = \"status:\"..code\n"
  "local subs_key = 'channel:subscribers:'..id\n"
  "local chan_key = 'channel:'..id\n"
  "\n"
  "for k,channel_key in pairs(redis.call('SMEMBERS', subs_key)) do\n"
  "  --not efficient, but useful for a few short-term subscriptions\n"
  "  redis.call('PUBLISH', channel_key, pubmsg)\n"
  "end\n"
  "--clear short-term subscriber list\n"
  "redis.call('DEL', subs_key)\n"
  "--now publish to the efficient channel\n"
  "redis.call('PUBLISH', channel_pubsub, pubmsg)\n"
  "return redis.call('HGET', chan_key, 'subscribers') or 0",

  //subscriber_register
  "--input: keys: [], values: [channel_id, subscriber_id, channel_empty_ttl, active_ttl, concurrency]\n"
  "--  'subscriber_id' can be '-' for new id, or an existing id\n"
  "--  'active_ttl' is channel ttl with non-zero subscribers. -1 to persist, >0 ttl in sec\n"
  "--  'concurrency' can be 'FIFO', 'FILO', or 'broadcast'\n"
  "--output: subscriber_id, num_current_subscribers\n"
  "\n"
  "local id, sub_id, active_ttl, concurrency = ARGV[1], ARGV[2], tonumber(ARGV[3]) or 20, ARGV[4]\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "if on then return function(...) redis.call('echo', table.concat({...})); end\n"
  "  else return function(...) return; end end\n"
  "end)(enable_debug)\n"
  "\n"
  "dbg(' ######## SUBSCRIBER REGISTER SCRIPT ####### ')\n"
  "\n"
  "local keys = {\n"
  "  channel =     'channel:'..id,\n"
  "  messages =    'channel:messages:'..id,\n"
  "  subscribers = 'channel:subscribers:'..id,\n"
  "  subscriber_id='channel:next_subscriber_id:'..id --integer\n"
  "}\n"
  "\n"
  "local setkeyttl=function(ttl)\n"
  "  for i,v in pairs(keys) do\n"
  "    if ttl > 0 then\n"
  "      redis.call('expire', v, ttl)\n"
  "    else\n"
  "      redis.call('persist', v)\n"
  "    end\n"
  "  end\n"
  "end\n"
  "\n"
  "--[[\n"
  "local check_concurrency_in = function(i, id)\n"
  "  if concurrency == \"FIFO\" then\n"
  "    return i==1 and id or \"DENY\"\n"
  "  end\n"
  "  return id\n"
  "end\n"
  "\n"
  "if concurrency == \"FILO\" then\n"
  "  --kick out old subscribers\n"
  "  \n"
  "  \n"
  "end\n"
  "]]\n"
  "\n"
  "local sub_count\n"
  "\n"
  "if sub_id == \"-\" then\n"
  "  sub_id =tonumber(redis.call('INCR', keys.subscriber_id))\n"
  "  sub_count=redis.call('hincrby', keys.channel, 'subscribers', 1)\n"
  "else\n"
  "  sub_count=redis.call('hget', keys.channel, 'subscribers')\n"
  "end\n"
  "setkeyttl(active_ttl)\n"
  "\n"
  "dbg(\"id= \", tostring(sub_id), \"count= \", tostring(sub_count))\n"
  "\n"
  "return {sub_id, sub_count}",

  //subscriber_unregister
  "--input: keys: [], values: [channel_id, subscriber_id, empty_ttl]\n"
  "-- 'subscriber_id' is an existing id\n"
  "-- 'empty_ttl' is channel ttl when without subscribers. 0 to delete immediately, -1 to persist, >0 ttl in sec\n"
  "--output: subscriber_id, num_current_subscribers\n"
  "\n"
  "local id, sub_id, empty_ttl = ARGV[1], ARGV[2], tonumber(ARGV[3]) or 20\n"
  "\n"
  "local enable_debug=true\n"
  "local dbg = (function(on)\n"
  "if on then return function(...) redis.call('echo', table.concat({...})); end\n"
  "  else return function(...) return; end end\n"
  "end)(enable_debug)\n"
  "\n"
  "dbg(' ######## SUBSCRIBER UNREGISTER SCRIPT ####### ')\n"
  "\n"
  "local keys = {\n"
  "  channel =     'channel:'..id,\n"
  "  messages =    'channel:messages:'..id,\n"
  "  subscribers = 'channel:subscribers:'..id,\n"
  "  subscriber_id='channel:next_subscriber_id:'..id --integer\n"
  "}\n"
  "\n"
  "local setkeyttl=function(ttl)\n"
  "  for i,v in pairs(keys) do\n"
  "    if ttl > 0 then\n"
  "      redis.call('expire', v, ttl)\n"
  "    elseif ttl < 0 then\n"
  "      redis.call('persist', v)\n"
  "    else\n"
  "      redis.call('del', v)\n"
  "    end\n"
  "  end\n"
  "end\n"
  "\n"
  "local sub_count = 0\n"
  "if redis.call('EXISTS', keys.channel) ~= 0 then\n"
  "   sub_count = redis.call('hincrby', keys.channel, 'subscribers', -1)\n"
  "\n"
  "  if sub_count == 0 then\n"
  "    setkeyttl(empty_ttl)\n"
  "  elseif sub_count < 0 then\n"
  "    return {err=\"Subscriber count for channel \" .. id .. \" less than zero: \" .. sub_count}\n"
  "  end\n"
  "else\n"
  "  dbg(\"channel \", id, \" already gone\")\n"
  "end\n"
  "\n"
  "return {sub_id, sub_count}"
};

