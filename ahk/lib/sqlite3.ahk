;
; sqlite3.ahk, 6/11/2021 1:14 AM
;

class sqlite3 extends AhkObj {

    __new(filename) {
        base.__new()
        this.filename := filename
        this.open(filename)
    }

    exec(sql, args*) {
        result := this.__Call("exec", Format(sql, args*))
        if (result.errcode)
            throw, Exception(Format("Sqlite3: {} (error code: {})`n`nSQL statement(s):`n     " sql
                            , result.errmsg, result.errcode, args*), -1)
        return result.length() > 0 ? result : ""
    }

    exists(sql, args*) {
        result := this.exec(sql, args*)
        return result.length() > 0
    }

    get(sql, args*) {
        result := this.exec(sql, args*)
        for name, value in result[1]
            return value
    }

    list(verbose = 0, all = false) {
        result := this.exec(all ? "
            (
                SELECT * FROM sqlite_schema ORDER BY 1;
            )" : "
            (
                SELECT * FROM sqlite_schema
                WHERE type IN ('table', 'view') AND name NOT LIKE 'sqlite_%'
                ORDER BY 1;
            )")

        if (result.length()) {
            tdata := result.length() " tables:`n"
            for i, t in result {
                if (verbose) {
                    columns := ", ("
                    for j, c in this.exec("PRAGMA table_info({});", t.tbl_name)
                        (A_Index == 1) ? columns .= c.name : columns .= ", " c.name
                    columns .= ")"
                }
                tdata .= Format("{:d}. {}, {}{}`n", i, t.name, t.type, columns)
                (verbose > 1) ? tdata .= t.sql "`n"
            }

            return tdata
        }
    }

    dump(table, limit = 10) {
        cols := this.exec("PRAGMA table_info({});", table)
        for i, c in cols
            cdata .= c.name "  "

        for n, r in this.exec("SELECT * FROM {} LIMIT {}", table, limit) {
            for i, c in cols
                (A_Index == 1) ? cdata .= "`n" r[c.name] : cdata .= ", " r[c.name]
        }
        cdata .= Format("`n...`nTotal {} rows.", this.get("SELECT count(*) FROM " table))

        return cdata
    }
}
