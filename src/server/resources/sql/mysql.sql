-- query:mgstpp.players.create
create table if not exists `players`
(
	id									bigint unsigned	not null	auto_increment,
	account_id							bigint unsigned	not null	 unique,
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
	resource_arrays			mediumblob default null,
	nuke_count				bigint unsigned default 0,
	staff_count				int unsigned not null,
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
	mb_coin					int default 0,
	version 				bigint unsigned default 0,
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
	cluster_param		json,
	construct_param		int unsigned	not null default 0,
	create_date			datetime        default current_timestamp not null,
	primary key (`id`),
	foreign key (`player_id`) references players(`id`)
)
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
