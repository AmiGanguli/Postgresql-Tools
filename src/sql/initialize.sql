
-- We drop the whole schema, so the 'drop table' and 'create or replace'
-- lage are redundant, but it's convenient to have them there anyway for
-- development.  Rather than running the entire script you can cut-
-- and-paste the modified section into a command window.
--
DROP SCHEMA IF EXISTS framework CASCADE;
CREATE SCHEMA framework;

DROP TABLE IF EXISTS framework.metadata;
CREATE TABLE framework.metadata (
	id	VARCHAR(16),
	value	TEXT,
	PRIMARY KEY(id)
);

CREATE OR REPLACE FUNCTION framework.init()
RETURNS VOID AS $$
BEGIN
END;
$$ LANGUAGE plpgsql;

CREATE  FUNCTION framework.reset()
RETURNS VOID AS $$
BEGIN
	DELETE FROM framework.metadata;
END;
$$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION framework.define_authentication_table(
	schema 			TEXT DEFAULT 'auth',
	user_table		TEXT DEFAULT 'users',
	user_table_id		TEXT DEFAULT 'id',
	user_table_username	TEXT DEFAULT 'username',
	user_table_pwhash	TEXT DEFAULT 'pwhash',
	user_table_
	table_name_ TEXT DEFAULT 
