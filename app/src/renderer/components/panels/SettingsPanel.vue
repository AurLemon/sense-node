<script setup lang="ts">
import { computed } from 'vue'
import AppIcon from '../ui/AppIcon.vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'

const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()

const expressionSource = computed(() => {
	if (device.currentFrame?.display_face) {
		return `${t('settings.expressionSourceDevice')} (${device.currentFrame.display_face})`
	}
	if (device.currentFrame?.final_event) {
		return `${t('settings.expressionSourceLocal')} (${device.currentFrame.final_event})`
	}
	return `${t('settings.expressionSourceLocal')} (${device.stableEvent})`
})
</script>

<template>
	<div class="grid gap-2.5">
		<label
			class="grid min-h-8 grid-cols-[auto_1fr] items-center gap-2 text-sm text-default"
		>
			<UCheckbox v-model="settings.autoReconnect" />
			<span>{{ t('settings.autoReconnect') }}</span>
		</label>
		<label
			class="grid min-h-8 grid-cols-[auto_1fr] items-center gap-2 text-sm text-default"
		>
			<UCheckbox v-model="settings.eventStabilizerEnabled" />
			<span>{{ t('settings.stabilizer') }}</span>
		</label>
		<div class="grid min-w-0 gap-1.5">
			<label class="text-xs text-muted">{{ t('common.language') }}</label>
			<USelect
				v-model="settings.locale"
				:items="[
					{ value: 'zh-CN', label: t('locale.zh') },
					{ value: 'en-US', label: t('locale.en') },
				]"
			/>
		</div>
		<div class="grid min-w-0 gap-1.5">
			<label class="text-xs text-muted">{{ t('common.theme') }}</label>
			<USelect
				v-model="settings.themeMode"
				:items="[
					{ value: 'dark', label: t('theme.dark') },
					{ value: 'light', label: t('theme.light') },
				]"
			/>
		</div>
		<div class="grid min-w-0 gap-1.5">
			<label class="text-xs text-muted">{{ t('settings.accent') }}</label>
			<UInput v-model="settings.themeAccent" type="color" />
		</div>
		<div class="grid min-w-0 gap-1.5">
			<label class="text-xs text-muted">{{
				t('settings.expressionSource')
			}}</label>
			<div class="min-h-8 content-center text-sm text-default">
				{{ expressionSource }}
			</div>
		</div>
		<div
			class="grid min-w-0 gap-1.5 mt-10 text-slate-400 dark:text-slate-500 text-sm"
		>
			<div>{{ t('settings.about.author') }}</div>
			<div>{{ t('settings.about.disclaimer') }}</div>
			<div class="inline-flex items-center gap-1.5">
				<AppIcon name="lucide:bot" :size="14" class="inline-block" />
				<span>{{ t('settings.about.agent') }}</span>
			</div>
		</div>
	</div>
</template>
