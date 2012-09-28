@load base/frameworks/intel

export {
	redef enum Intel::Where += {
		HTTP::IN_USER_AGENT_HEADER,
	};
}

event http_header(c: connection, is_orig: bool, name: string, value: string)
	{
	if ( is_orig && name == "USER-AGENT" )
		Intel::seen([$str=value,
		             $str_type=Intel::USER_AGENT,
		             $conn=c,
		             $where=HTTP::IN_USER_AGENT_HEADER]);
	}
