import { createPinia } from 'pinia'
import { createApp } from 'vue'
import App from './renderer/App.vue'
import './renderer/styles/app.css'

createApp(App).use(createPinia()).mount('#app')
