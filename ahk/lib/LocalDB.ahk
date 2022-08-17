;
; LocalDB.ahk, 9/10/2021 11:07 AM
;

global __translations := {}
global __apiEndpoints := { "en": "https://www.pathofexile.com/api/trade/data"
                         , "zh-CN": "https://poe.game.qq.com/api/trade/data"
                         , "fr": "https://fr.pathofexile.com/api/trade/data"
                         , "ru": "https://ru.pathofexile.com/api/trade/data"
                         , "de": "https://de.pathofexile.com/api/trade/data"
                         , "es": "https://es.pathofexile.com/api/trade/data"
                         , "th": "https://th.pathofexile.com/api/trade/data"
                         , "pt-BR": "https://br.pathofexile.com/api/trade/data"
                         , "ko-KR": "https://poe.game.daum.net/api/trade/data" }

class LocalDB extends sqlite3 {

    static version := "1.0"

    __new(filename) {
        base.__new(filename)
        this.exec("PRAGMA journal_mode=MEMORY;")
        this.exec("PRAGMA threads=5;")

        if (this.load("db_version") != this.version) {
            this.initialze()
            this.store("db_version", this.version, "All")
        }
        this.loadTranslations()
    }

    load(name) {
        try {
            return this.get("
                (
                SELECT value FROM properties
                WHERE name='{}' AND (league='All' OR league='{}');
                )", name, ptask.league)
        } catch {}
    }

    store(name, value, league = "") {
        (not league) ? league := ptask.league
        InStr(value, "'") ? value := StrReplace(value, "'", "''")
        this.exec("INSERT OR REPLACE INTO properties VALUES ('{}', '{}', '{}');"
                 , name, value, league)
    }

    delete(name, league = "") {
        (not league) ? league := ptask.league
        this.exec("DELETE FROM properties WHERE name LIKE '{}' AND league='{}';"
                 , name, league)
    }

    addTranslation(source, target) {
        if (language == "en" || not target || __translations[source] == target)
            return
        __translations[source] := target
        this.exec("INSERT INTO v_trans VALUES (""{}"", ""{}"", '{}');"
                 , source, target, language)
    }

    loadTranslations() {
        this.exec("
            (
            DROP VIEW IF EXISTS v_trans;
            CREATE VIEW v_trans AS
                SELECT
                    literals.text AS source,
                    translations.text AS target,
                    language
                FROM translations INNER JOIN literals USING (id)
                WHERE language='{}';
            
            CREATE TRIGGER IF NOT EXISTS t_add_translation
            INSTEAD OF INSERT ON v_trans
            BEGIN
                INSERT INTO literals (text)
                VALUES (NEW.source) ON CONFLICT (text) DO NOTHING;

                INSERT INTO translations
                VALUES ((SELECT id FROM literals WHERE text=NEW.source),
                        NEW.target, NEW.language)
                    ON CONFLICT (id, language) DO NOTHING;
            END;
            )", language)

        __translations := {}
        if (language == "en")
            return

        for i, r in this.exec("SELECT * FROM v_trans;")
            __translations[r.source] := r.target

        dict := JSON.load("lib\translations.json")
        for i, t in dict[language]
            t ? __translations[dict["en"][i]] := t

        ; load static item names from PoE website
        staticItemNames := {}
        for i, result in JSON.parse(ajax(__apiEndpoints[language] "/static")).result {
            for j, entry in result.entries
                staticItemNames[entry.id] := entry
        }

        for i, result in JSON.parse(ajax(__apiEndpoints["en"] "/static")).result {
            for j, entry in result.entries
                this.addTranslation(entry.text, staticItemNames[entry.id].text)
        }

        trace("Loading translations... {}", __translations.count())
    }

    initialze() {
        rdebug("#LocalDB", "Initializing database (version {})...", this.version)
        this.exec("
            (
            DROP TABLE IF EXISTS properties;
            CREATE TABLE properties (
                name TEXT COLLATE NOCASE,
                value TEXT,
                league TEXT COLLATE NOCASE,
                PRIMARY KEY (name, league));

            DROP TABLE IF EXISTS literals;
            CREATE TABLE literals (
                id INTEGER PRIMARY KEY,
                text TEXT UNIQUE);

            DROP TABLE IF EXISTS translations;
            CREATE TABLE translations (
                id INTEGER,
                text TEXT,
                language TEXT,
                PRIMARY KEY (id, language));
            )")
        rdebug("#LocalDB", "Initializing database (version {})... Done!", this.version)
    }
}
