import vue from '@vitejs/plugin-vue'
import ui from '@nuxt/ui/vite'
import { defineConfig } from 'vite'

// https://vitejs.dev/config
export default defineConfig({
	plugins: [
		vue(),
		ui({
			ui: {
				colors: {
					primary: 'sky',
					secondary: 'slate',
					success: 'emerald',
					info: 'sky',
					warning: 'amber',
					error: 'red',
					neutral: 'zinc',
				},
				icons: {
					loading: 'i-lucide-loader-circle',
					search: 'i-lucide-search',
					menu: 'i-lucide-menu',
					close: 'i-lucide-x',
					check: 'i-lucide-check',
					minus: 'i-lucide-minus',
				},
			},
		}),
	],
})
