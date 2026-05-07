import { createPinia } from 'pinia'
import { createMemoryHistory, createRouter } from 'vue-router'
import ui from '@nuxt/ui/vue-plugin'
import { addCollection } from '@iconify/vue'
import lucideIcons from '@iconify-json/lucide/icons.json'
import { createApp } from 'vue'
import App from './renderer/App.vue'
import 'overlayscrollbars/overlayscrollbars.css'
import './renderer/styles/app.css'
import './renderer/styles/fonts.css'
import './renderer/styles/tailwind.css'

addCollection(lucideIcons)

const router = createRouter({
	history: createMemoryHistory(),
	routes: [],
})

createApp(App).use(createPinia()).use(router).use(ui).mount('#app')
