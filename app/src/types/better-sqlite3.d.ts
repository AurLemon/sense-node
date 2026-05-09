declare module 'better-sqlite3' {
	namespace Database {
		interface RunResult {
			changes: number
			lastInsertRowid: number | bigint
		}

		interface Statement {
			run(...params: unknown[]): RunResult
			all(...params: unknown[]): unknown[]
		}

		interface Database {
			pragma(source: string): unknown
			exec(source: string): void
			prepare(source: string): Statement
		}
	}

	interface DatabaseConstructor {
		new (filename: string): Database.Database
		(filename: string): Database.Database
	}

	const Database: DatabaseConstructor
	export = Database
}
