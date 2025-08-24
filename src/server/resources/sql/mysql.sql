-- query:mgstpp.players.create
create table if not exists `players`
(
	id									bigint unsigned	not null	auto_increment,
	account_id							bigint unsigned				 unique,
	session_id							char(32)		default null unique,
	login_password						char(32)		default null,
	last_update							datetime		default null,
	crypto_key							char(32)		default null,
	smart_device_id						char(128)		default null,
	currency							varchar(32)		default null,
	ex_ip								varchar(15)		default null,
	in_ip								varchar(15)		default null,
	ex_port								int unsigned	default 0,
	in_port								int unsigned	default 0,
	nat									int unsigned	default 0,
	creation_time						datetime        not null,
	security_challenge					boolean			not null default false,
	current_lock						bigint unsigned	not null		default 0,
	current_sneak_mode					int unsigned	not null		default 0,
	current_sneak_fob					bigint unsigned not null		default 0,
	current_sneak_player				bigint unsigned not null		default 0,
	current_sneak_platform				int unsigned	not null		default 0,
	current_sneak_status				int unsigned	not null		default 0,
	current_sneak_is_sneak				tinyint			not null		default 0,
	current_sneak_security_challenge 	tinyint			not null		default 0,
	current_sneak_start 				datetime,
	primary key (`id`)
)
-- query:mgstpp.players.set_auto_increment
alter table `{}`.`players` auto_increment = {};
-- query:mgstpp.player_records.create
create table if not exists `player_records`
(
	id							bigint unsigned	not null	auto_increment,
	player_id					bigint unsigned	not null,
	fob_grade					int not null	default 0,
	prev_fob_grade				int not null	default 0,
	fob_point					int not null	default 0,
	fob_rank					int not null	default 0,
	prev_fob_rank				int not null	default 0,
	is_insurance				boolean			default false,
	league_grade				int not null	default 0,
	prev_league_grade			int not null	default 0,
	league_rank					int not null	default 0,
	prev_league_rank			int not null	default 0,
	league_point				int not null	default 0,
	event_point					int not null	default 0,
	playtime					int not null	default 0,
	point						int not null	default 0,
	fob_defense_win				int not null	default 0,
	fob_defense_lose			int not null	default 0,
	fob_sneak_win				int not null	default 0,
	fob_sneak_lose				int not null	default 0,
	fob_deploy_emergency_count	int not null	default 0,
	shield_date					datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.player_records.update_fob_ranking
with ranked_players as (
	select player_id, fob_point, 
		   (select count(distinct fob_point) + 1 
			from player_records pr2 
			where pr2.fob_point > pr1.fob_point) as new_rank
	from player_records pr1
)
update player_records record
join ranked_players ranked_player on record.player_id = ranked_player.player_id
set record.fob_rank = ranked_player.new_rank where record.fob_point > 0;
-- query:mgstpp.player_data.create
create table if not exists `player_data`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	unit_counts				blob default null,
	unit_levels				blob default null,
	resource_arrays			blob default null,
	nuke_count				bigint unsigned default 0,
	staff_count				int unsigned not null,
	staff_counts			blob default null,
	staff_bin				mediumblob default null,
	loadout					json not null,
	motherbase				json not null,
	emblem					json not null,
	local_gmp				int default 0,
	server_gmp				int default 0,
	loadout_gmp				int default 0,
	insurance_gmp			int default 0,
	injury_gmp				int default 0,
	last_sync				datetime default null,
	mb_coin					int unsigned default 0,
	client_resource_version bigint unsigned default 0,
	client_staff_version 	bigint unsigned default 0,
	server_resource_version bigint unsigned default 0,
	server_staff_version 	bigint unsigned default 0,
	fob_deploy_damage_param json,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.items.create
create table if not exists `items`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id           bigint unsigned	not null,
	item_id				bigint unsigned	not null,
	create_date			datetime        default current_timestamp not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.player_follows.create
create table if not exists `player_follows`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`),
	unique key `unique_player_follow` (`player_id`, `to_player_id`),
    check (`player_id` != `to_player_id`)
)
-- query:mgstpp.fobs.create
create table if not exists `fobs`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	fob_index			bigint unsigned	not null,
	platform_count		int	unsigned	not null default 0,
	security_rank		int unsigned	not null default 0,
	area_id	            int unsigned	not null default 0,
	cluster_param		blob					 default null,
	construct_param		int unsigned	not null default 0,
	create_date			datetime        default current_timestamp not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.fobs.set_auto_increment
alter table `{}`.`fobs` auto_increment = {};
-- query:mgstpp.sneak_results.create
create table if not exists `sneak_results`
(
	id                  bigint unsigned		not null	auto_increment,
	player_id	        bigint unsigned		not null,
	target_id	        bigint unsigned		not null,
	fob_id				bigint unsigned		not null,
	fob_index			bigint unsigned		not null,
	is_win				tinyint unsigned	not null,
	platform			int unsigned 		not null,
	data				json,
	create_date			datetime not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`target_id`) references players(`id`),
	foreign key (`fob_id`) references fobs(`id`)
)
-- query:mgstpp.wormholes.create
create table if not exists `wormholes`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	retaliate_score	    int unsigned	not null	default 0,
	flag				int unsigned	not null	default 0,
	is_open				boolean			not null	default false,
	create_date			datetime		not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`)
)
-- query:mgstpp.event_rankings.create
create table if not exists `event_rankings`
(
	id                  bigint unsigned	not null	auto_increment,
	player_id	        bigint unsigned	not null,
	event_id	        int unsigned	not null,
	player_rank		    bigint unsigned	not null default 0,
	value		        int unsigned	not null default 0,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique key `unique_player_event_rankings_type` (`player_id`, `event_id`)
)
-- query:mgstpp.event_rankings.update_entries
update event_rankings event_ranking
join (
	select
		player_id,
		event_id,
		value,
		case 
			when value = 0 then 0 
			else rank() over (partition by event_id order by value desc)
		end as new_rank
	from event_rankings
) ranked
on event_ranking.player_id = ranked.player_id AND event_ranking.event_id = ranked.event_id
set event_ranking.player_rank = ranked.new_rank;
-- query:mgstpp.mgo_characters.create
create table if not exists `mgo_characters`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	character_index			int unsigned	not null,
	avatar					json not null,
	loadouts				json not null,
	last_loadout			int unsigned not null default 0,
	name					char(32),
	player_class			int unsigned not null default 0,
	player_type				int unsigned  not null default 0,
	legendary				int unsigned  not null default 0,
	prestige				int unsigned  not null default 0,
	xp						int unsigned  not null default 0,
	permanent_unlock_list   int unsigned  not null default 0,
	create_date				datetime		default current_timestamp not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.mgo_data.create
create table if not exists `mgo_data`
(
	id							bigint unsigned	not null	auto_increment,
	player_id					bigint unsigned	not null,
	last_character_used			int unsigned default 0 not null,
	bgm_selected				int unsigned default 0 not null,
	match_auto_leave			int unsigned default 0 not null,
	match_briefing_time			int unsigned default 0 not null,
	match_host_comment			int unsigned default 0 not null,
	match_max_capacity			int unsigned default 0 not null,
	match_mission_slot_count	int unsigned default 0 not null,
	gp_coin						int unsigned default 0 not null,
	gp_boost_mag				int unsigned default 0 not null,
	gp_expire_unix_timestamp	int unsigned default 0 not null,
	rank_xp						int unsigned default 0 not null,
	xp_boost_mag				int unsigned default 0 not null,
	xp_expire_unix_timestamp	int unsigned default 0 not null,
	reward_category				int unsigned default 0 not null,
	reward_id_a					int unsigned default 0 not null,
	reward_id_b					int unsigned default 0 not null,
	reward_id_c					int unsigned default 0 not null,
	survival_ticket_remain		int unsigned default 0 not null,
	mission_slot_list			int unsigned default 0 not null,
	mission_player_num			int unsigned default 0 not null,
	matches_played				int unsigned default 0 not null,
	matches_abandoned			int unsigned default 0 not null,
	matches_started				int unsigned default 0 not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique key `unique_mgo_data_player_id` (`player_id`)
)
-- query:mgstpp.mgo_stats.create
create table if not exists `mgo_stats`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	stat_id					int unsigned	not null,
	stat_value				int unsigned	not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique key `unique_mgo_stat_player_id_stat_id` (`player_id`, `stat_id`)
)
-- query:mgstpp.mgo_color_purchase.create
create table if not exists `mgo_color_purchase`
(
	id						bigint unsigned	not null	auto_increment,
	player_id				bigint unsigned	not null,
	gear_id					int unsigned	not null,
	color_id				int unsigned	not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`),
	unique key `unique_mgo_color_purchase_player_id_gear_id_color_id` (`player_id`, `gear_id`, `color_id`)
)
-- query:mgstpp.variables.create
create table if not exists `variables`
(
	id						bigint unsigned	not null	auto_increment,
	variable_name			varchar(256) 	not null	unique,
	variable_value			json			not null,
	primary key (`id`)
)
