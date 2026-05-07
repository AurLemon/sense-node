<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import { motion } from 'motion-v'
import { useToast } from '@nuxt/ui/composables'
import { OverlayScrollbarsComponent } from 'overlayscrollbars-vue'
import BottomTabs from './components/layout/BottomTabs.vue'
import SenseNodeScene from './components/three/SenseNodeScene.vue'
import DeviceStatusPanel from './components/panels/DeviceStatusPanel.vue'
import EventManagementPanel from './components/panels/EventManagementPanel.vue'
import SettingsPanel from './components/panels/SettingsPanel.vue'
import { useI18n } from './lib/i18n'
import { stableEvents } from '../shared/types/sensenode'
import { useEventStore } from './stores/eventStore'
import { useDeviceStore } from './stores/deviceStore'
import { useSerialStore } from './stores/serialStore'
import { useSettingsStore } from './stores/settingsStore'

const device = useDeviceStore()
const events = useEventStore()
const serial = useSerialStore()
const settings = useSettingsStore()
const { t } = useI18n()
const toast = useToast()
const activeTab = ref('scene')
let cleanupListeners: Array<() => void> = []
const isConnected = computed(() => serial.status.state === 'connected')
const statusDotStyle = computed(() =>
	isConnected.value
		? {
				backgroundColor: '#22c55e',
				boxShadow: '0 0 20px 0 rgba(34, 197, 94, 0.7)',
			}
		: {
				backgroundColor: '#ef4444',
				boxShadow: '0 0 20px 0 rgba(239, 68, 68, 0.7)',
			},
)

const tabComponent = computed(() => {
	if (activeTab.value === 'device') return DeviceStatusPanel
	if (activeTab.value === 'events') return EventManagementPanel
	if (activeTab.value === 'settings') return SettingsPanel
	return SenseNodeScene
})

const tabSectionClass = computed(() =>
	activeTab.value === 'scene'
		? 'absolute inset-0 z-0'
		: 'absolute inset-x-0 top-12 bottom-16 z-0 overflow-hidden',
)

const tabContentClass = computed(() =>
	activeTab.value === 'scene'
		? 'h-full w-full'
		: 'h-full w-full px-3.5 pt-8 pb-24',
)

const overlayScrollOptions = {
	scrollbars: {
		theme: 'os-theme-sensenode',
		autoHide: 'leave',
		autoHideDelay: 450,
		clickScroll: true,
	},
}

events.loadTasks()
let firstStableEvent = true

watch(
	() => device.stableEvent,
	(event) => {
		if (firstStableEvent) {
			firstStableEvent = false
			return
		}
		void events.notifyStableEvent(event)
	},
	{ immediate: true },
)

watch(
	() => settings.eventStabilizerEnabled,
	(enabled) => {
		device.stabilizerEnabled = enabled
	},
	{ immediate: true },
)

watch(
	() => settings.locale,
	(locale) => {
		document.documentElement.lang = locale
	},
	{ immediate: true },
)

watch(
	() => settings.themeMode,
	(themeMode) => {
		document.documentElement.dataset.theme = themeMode
		document.documentElement.style.colorScheme = themeMode
		document.documentElement.classList.toggle('dark', themeMode === 'dark')
		document.documentElement.classList.toggle('light', themeMode === 'light')
	},
	{ immediate: true },
)

onMounted(() => {
	cleanupListeners = [
		window.sensenode.device.onFrame((frame) => {
			serial.notifyFrame(frame)
			device.ingestFrame(frame, 'serial')
		}),
		window.sensenode.device.onStatus((status) => {
			serial.status = status
		}),
		window.sensenode.serial.onLine(serial.pushLine),
	]

	void serial.autoConnectUsb().then((connected) => {
		if (connected) {
			return
		}

		toast.add({
			title: t('serial.autoConnectFailed'),
			description: t('serial.autoConnectFailedDescription'),
			icon: 'i-lucide-circle-alert',
			color: 'warning',
		})
	})
})

onBeforeUnmount(() => {
	for (const cleanup of cleanupListeners) cleanup()
	cleanupListeners = []
})
</script>

<template>
	<UApp :toaster="{ class: 'z-[9999]' }">
		<main class="relative h-screen overflow-hidden bg-default text-default">
			<header class="sticky left-0 top-0 right-0 z-[1] h-12">
				<div
					class="pointer-events-none absolute left-0 right-0 top-0 -bottom-8 bg-[#FAFAFA]/80 backdrop-blur-[40px] mask-[linear-gradient(to_bottom,black_0%,rgba(0,0,0,0.96)_32%,rgba(0,0,0,0.78)_52%,rgba(0,0,0,0.42)_72%,transparent_100%)] dark:bg-[#192024]/85"
				/>
				<div
					class="relative z-[1] flex h-full items-center justify-between gap-4 px-4 py-4"
				>
					<div class="min-w-0">
						<div
							class="flex items-center gap-3 text-xl font-semibold leading-none"
						>
							<motion.span
								:key="serial.status.state"
								class="inline-block h-2.5 w-2.5 rounded-full"
								:style="statusDotStyle"
								:initial="{ scale: 0.6, opacity: 0.3 }"
								:animate="{ scale: [0.85, 1.18, 1], opacity: 1 }"
								:transition="{
									duration: 0.38,
									times: [0, 0.55, 1],
									ease: 'easeOut',
								}"
							/>
							<span>
								{{ t('app.title') }}
							</span>
						</div>
					</div>
					<div class="flex items-center gap-2">
						<UBadge
							v-if="isConnected"
							color="link"
							variant="soft"
							class="text-xs uppercase"
						>
							{{ t(`events.event.${device.stableEvent}`) }} ·
							{{ t('app.fps') }}
							{{ serial.status.fps }} · {{ t('app.inference') }}
							{{ device.currentFrame?.inference_ms ?? '-' }} ms
						</UBadge>
						<span v-else class="text-xs text-muted">
							{{ t('app.disconnected') }}
						</span>
					</div>
				</div>
			</header>

			<Transition name="tab-fade" mode="out-in">
				<section :key="activeTab" :class="tabSectionClass">
					<OverlayScrollbarsComponent
						v-if="activeTab !== 'scene'"
						defer
						class="h-full w-full"
						:options="overlayScrollOptions"
					>
						<div :class="tabContentClass">
							<component :is="tabComponent" />
						</div>
					</OverlayScrollbarsComponent>
					<div v-else :class="tabContentClass">
						<component :is="tabComponent" />
					</div>
				</section>
			</Transition>

			<footer class="fixed bottom-0 left-0 right-0 z-[1] h-16">
				<div
					class="pointer-events-none absolute bottom-0 left-0 right-0 -top-20 bg-[#FAFAFA]/80 backdrop-blur-[48px] mask-[linear-gradient(to_top,black_0%,rgba(0,0,0,0.98)_30%,rgba(0,0,0,0.92)_45%,rgba(0,0,0,0.8)_55%,rgba(0,0,0,0.58)_65%,rgba(0,0,0,0.35)_75%,rgba(0,0,0,0.15)_85%,transparent_100%)] dark:bg-[#192024]/85"
				/>
				<BottomTabs v-model="activeTab" class="relative z-[1]" />
			</footer>
		</main>
	</UApp>
</template>
