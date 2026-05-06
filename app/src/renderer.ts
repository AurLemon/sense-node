import { createPinia } from 'pinia'
import ui from '@nuxt/ui/vue-plugin'
import { addCollection } from '@iconify/vue'
import lucideIcons from '@iconify-json/lucide/icons.json'
import { createApp } from 'vue'
import App from './renderer/App.vue'
import './renderer/styles/app.css'
import './renderer/styles/fonts.css'
import './renderer/styles/tailwind.css'

addCollection(lucideIcons)

createApp(App).use(createPinia()).use(ui).mount('#app')
