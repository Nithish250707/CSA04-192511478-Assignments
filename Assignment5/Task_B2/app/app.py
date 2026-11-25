"""University Result Portal – a small Flask app used for CSA04 Assignment 5 (B1–B4).

Database: PostgreSQL when DATABASE_URL is set (Docker Compose), otherwise a local
SQLite file (bare-metal runs). Both are seeded with the same 1,000 made-up demo students
(register numbers 20260000-20260999; grades are random, not real).
"""
import os
import random
import socket
import sqlite3

from flask import Flask, abort, jsonify

app = Flask(__name__)
DATABASE_URL = os.environ.get("DATABASE_URL")
SQLITE_PATH = os.environ.get("SQLITE_PATH", os.path.join(os.path.dirname(os.path.abspath(__file__)), "portal.db"))
SUBJECTS = ["CSA04 Operating Systems", "CSA14 Compiler Design", "UBA06 Mathematics", "EEA01 Electrical"]


def connect():
    if DATABASE_URL:
        import psycopg  # only needed for the Compose/PostgreSQL set-up
        return psycopg.connect(DATABASE_URL), "%s"
    return sqlite3.connect(SQLITE_PATH, timeout=10), "?"


def init_db():
    conn, ph = connect()
    with conn:
        cur = conn.cursor()
        cur.execute("CREATE TABLE IF NOT EXISTS results (reg_no TEXT, subject TEXT, grade TEXT, "
                    "PRIMARY KEY (reg_no, subject))")
        cur.execute("SELECT COUNT(*) FROM results")
        if cur.fetchone()[0] == 0:
            rng = random.Random(42)
            rows = [(str(20260000 + i), s, rng.choice("SABCDE"))
                    for i in range(1000) for s in SUBJECTS]
            sql = f"INSERT INTO results VALUES ({ph}, {ph}, {ph}) ON CONFLICT DO NOTHING"
            cur.executemany(sql, rows)
    conn.close()


init_db()


@app.get("/")
def index():
    return ("<h1>University Result Portal</h1>"
            "<p>GET /result/&lt;register-number&gt; for results, e.g. "
            "<a href='/result/20260001'>/result/20260001</a></p>"
            f"<p>Served by {socket.gethostname()}</p>")


@app.get("/health")
def health():
    return jsonify(status="ok", db="postgres" if DATABASE_URL else "sqlite")


@app.get("/result/<reg_no>")
def result(reg_no):
    conn, ph = connect()
    try:
        cur = conn.cursor()
        cur.execute(f"SELECT subject, grade FROM results WHERE reg_no = {ph} ORDER BY subject", (reg_no,))
        rows = cur.fetchall()
    finally:
        conn.close()
    if not rows:
        abort(404)
    return jsonify(reg_no=reg_no, results=[{"subject": s, "grade": g} for s, g in rows],
                   served_by=socket.gethostname())
