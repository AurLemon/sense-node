<script setup lang="ts">
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'

const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()
</script>

<template>
	<div class="grid gap-3">
		<div
			class="grid grid-cols-2 gap-3 rounded-lg border border-default bg-default/75 p-3 shadow-sm backdrop-blur"
		>
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
			<label
				class="grid min-h-[34px] grid-cols-[auto_1fr] items-center gap-2.5 text-xs text-muted"
			>
				<UCheckbox v-model="settings.miniHudEnabled" />
				<span>{{ t('settings.miniHud') }}</span>
			</label>
			<label
				class="grid min-h-[34px] grid-cols-[auto_1fr] items-center gap-2.5 text-xs text-muted"
			>
				<UCheckbox v-model="settings.autoReconnect" />
				<span>{{ t('settings.autoReconnect') }}</span>
			</label>
			<label
				class="grid min-h-[34px] grid-cols-[auto_1fr] items-center gap-2.5 text-xs text-muted"
			>
				<UCheckbox v-model="settings.eventStabilizerEnabled" />
				<span>{{ t('settings.stabilizer') }}</span>
			</label>
			<div class="grid min-w-0 gap-1.5">
				<label class="text-xs text-muted">{{ t('settings.accent') }}</label>
				<UInput v-model="settings.themeAccent" type="color" />
			</div>
		</div>

		<div
			class="rounded-lg border border-default bg-default/75 p-3 font-['JetBrains_Mono','MiSans',monospace] text-muted shadow-sm backdrop-blur"
		>
			{{ t('settings.currentStable') }}:
			{{ device.currentFrame ? device.stableEvent : '-' }}
		</div>
	</div>
</template>
