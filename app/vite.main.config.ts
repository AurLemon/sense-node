import { defineConfig } from 'vite'

// https://vitejs.dev/config
export default defineConfig({
	build: {
		rollupOptions: {
			external: [
				'@serialport/bindings-cpp',
				'@serialport/parser-readline',
				'@serialport/stream',
				'better-sqlite3',
			],
		},
	},
})
