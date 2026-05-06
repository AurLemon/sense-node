<script setup lang="ts">
import { computed, ref } from 'vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'

const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()
const prompt = ref('请总结当前 SenseNode 的状态。')
const result = ref('')

const providerItems = [
	{ value: 'none', label: 'none' },
	{ value: 'ollama', label: 'ollama' },
	{ value: 'openai-compatible', label: 'openai-compatible' },
]

const summary = computed(() => {
	return device.frames
		.slice(0, 200)
		.map((frame) => frame.final_event ?? frame.type)
		.slice(0, 12)
		.join(' -> ')
})

function analyze(): void {
	result.value = `${t('lm.reserved')}: ${settings.llmConfig.provider}. ${t('lm.currentStable')}: ${device.stableEvent}.`
}
</script>

<template>
	<div class="h-full">
		<div
			class="grid grid-cols-3 gap-3 rounded-lg border border-default bg-default/75 p-3 shadow-sm backdrop-blur"
		>
			<div class="grid min-w-0 gap-1.5">
				<label class="text-xs text-muted">{{ t('lm.provider') }}</label>
				<USelect v-model="settings.llmConfig.provider" :items="providerItems" />
			</div>
			<div class="grid min-w-0 gap-1.5">
				<label class="text-xs text-muted">{{ t('lm.endpoint') }}</label>
				<UInput
					v-model="settings.llmConfig.endpoint"
					placeholder="http://127.0.0.1:11434"
				/>
			</div>
			<div class="grid min-w-0 gap-1.5">
				<label class="text-xs text-muted">{{ t('lm.model') }}</label>
				<UInput v-model="settings.llmConfig.model" placeholder="model-name" />
			</div>
			<div class="grid min-w-0 gap-1.5 col-span-full">
				<label class="text-xs text-muted">{{ t('lm.prompt') }}</label>
				<UTextarea v-model="prompt" :rows="4" />
			</div>
			<div class="grid min-w-0 gap-1.5 col-span-full">
				<label class="text-xs text-muted">{{ t('lm.summary') }}</label>
				<UTextarea :model-value="summary" readonly :rows="3" />
			</div>
			<div class="col-span-full grid gap-2.5">
				<UButton color="primary" variant="solid" type="button" @click="analyze">
					{{ t('lm.analyze') }}
				</UButton>
				<div
					class="break-words font-['JetBrains_Mono','MiSans',monospace] text-muted"
				>
					{{ result }}
				</div>
			</div>
		</div>
	</div>
</template>
